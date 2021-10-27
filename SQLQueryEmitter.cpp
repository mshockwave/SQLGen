#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringSwitch.h"
#include "Error.h"
#include "SQLQueryEmitter.h"

using namespace llvm;

namespace {
class SQLWhereClauseEmitter {
  raw_ostream &OS;

  enum OperatorKind {
    Unknown = 0,
    OPK_EQ,
    OPK_NE,
    OPK_GT,
    OPK_GE,
    OPK_LT,
    OPK_LE,
    OPK_AND,
    OPK_OR
  };

  OperatorKind getOperator(const Init *DagOperator) {
    auto OpString = DagOperator->getAsString();
    return StringSwitch<OperatorKind>(OpString)
            .Case("eq", OPK_EQ)
            .Case("ne", OPK_NE)
            .Case("gt", OPK_GT)
            .Case("ge", OPK_GE)
            .Case("lt", OPK_LT)
            .Case("le", OPK_LE)
            .Case("and", OPK_AND)
            .Case("or", OPK_OR)
            .Default(Unknown);
  }

  SmallVector<const Init *, 8> Nodes;
  void insertNode(unsigned Idx, const Init *Node) {
    if (Idx < Nodes.size())
      Nodes[Idx] = Node;
    else {
      Nodes.append(Idx - Nodes.size(), nullptr);
      Nodes.push_back(Node);
    }
  }

  // Elements in this set are index of Nodes whose `Init`
  // instance needs to be printed as quoted string.
  SmallSet<unsigned, 2> AsQuotedString;

public:
  SQLWhereClauseEmitter(raw_ostream &OS) : OS(OS) {}

  Error run(const DagInit *WhereClause);
};
} // end anonymous namespace

Error SQLWhereClauseEmitter::run(const DagInit *WhereClause) {

  // {The operand / operator Init, prospective index}
  SmallVector<std::pair<const Init *, unsigned>, 4> Worklist;

  Worklist.push_back({WhereClause, 0U});

  // First step, build the expression tree.
  const Init *Term;
  unsigned Idx;
  while (!Worklist.empty()) {
    std::tie(Term, Idx) = Worklist.pop_back_val();
    if (const auto *DagTerm = dyn_cast<DagInit>(Term)) {
      if (!getOperator(DagTerm->getOperator()))
        return createTGStringError(SMLoc(), "Unknown where clause operator");
      if (DagTerm->arg_size() != 2)
        return createTGStringError(SMLoc(), "Only binary operators are "
                                   "supported now");
      Term = DagTerm->getOperator();
      for (int i : {1, 0}) {
        // Also push the operands into the work list.
        Worklist.push_back({DagTerm->getArg(i), 2 * Idx + (i + 1)});
        if (const auto *Tag = DagTerm->getArgName(i)) {
          if (Tag->getValue() == "str")
            AsQuotedString.insert(2 * Idx + (i + 1));
        }
      }
    }
    insertNode(Idx, Term);
  }

  assert(Nodes.size());

  OS << "WHERE ";

  // Then, visit the tree in an infix fashion.
  SmallVector<unsigned, 4> VisitStack;
  VisitStack.push_back(0U);

  unsigned NumNodes = Nodes.size();
  while (!VisitStack.empty()) {
    unsigned Idx = VisitStack.back();
    unsigned LeftIdx = 2 * Idx + 1,
             RightIdx = 2 * Idx + 2;
    // Visit the left tree.
    if (LeftIdx < NumNodes && Nodes[LeftIdx]) {
      VisitStack.push_back(LeftIdx);
      continue;
    }

    // Print the root.
    assert(Nodes[Idx]);
    if (auto OpK = getOperator(Nodes[Idx])) {
      switch (OpK) {
      case OPK_NE:
        OS << " <> ";
        break;
      case OPK_EQ:
        OS << " = ";
        break;
      case OPK_LE:
        OS << " <= ";
        break;
      case OPK_LT:
        OS << " < ";
        break;
      case OPK_GE:
        OS << " >= ";
        break;
      case OPK_GT:
        OS << " > ";
        break;
      case OPK_OR:
        OS << " OR ";
        break;
      case OPK_AND:
        OS << " AND ";
        break;
      default:
        llvm_unreachable("Unrecognized operator kind");
      }
    } else {
      if (AsQuotedString.count(Idx))
        OS << Nodes[Idx]->getAsString();
      else
        OS << Nodes[Idx]->getAsUnquotedString();
    }
    VisitStack.pop_back();
    Nodes[Idx] = nullptr;

    // Visit the right tree.
    if (RightIdx < NumNodes && Nodes[RightIdx])
      VisitStack.push_back(RightIdx);
  }

  return Error::success();
}

Error SQLQueryEmitter::run(ArrayRef<const Record *> Queries) {
  // Map from the tag name to its corresponding SQL table field name.
  StringMap<StringRef> FieldTagMap;

  for (const auto *QueryRecord : Queries) {
    auto MaybeTableName = QueryRecord->getValueAsOptionalString("TableName");
    if (!MaybeTableName || MaybeTableName->empty())
      return createTGStringError(QueryRecord->getLoc()[0], "SQL table name "
                                 "is missing");
    auto TableName = *MaybeTableName;

    OS << "SELECT ";

    const DagInit *FieldsInit = QueryRecord->getValueAsDag("Fields");
    auto FieldOpName = FieldsInit->getOperator()->getAsString();
    if (FieldOpName == "all")
      OS << "*";
    else if (FieldOpName != "fields")
      // FIXME: This is a terrible SMLoc to use here.
      return createTGStringError(QueryRecord->getLoc()[0], "Invalid dag operator"
                                 " \"" + FieldOpName + "\"");

    FieldTagMap.clear();
    {
      ListSeparator LS;
      for (unsigned i = 0U; i != FieldsInit->arg_size(); ++i)
        if (const auto *Field = dyn_cast<StringInit>(FieldsInit->getArg(i))) {
          OS << LS << Field->getValue();
          if (const auto *Tag = FieldsInit->getArgName(i))
            FieldTagMap.insert({Tag->getValue(), Field->getValue()});
        }
    }

    OS << " FROM " << TableName;

    const DagInit *WhereClause = QueryRecord->getValueAsDag("WhereClause");
    if (WhereClause->getOperator()->getAsString() != "none") {
      OS << "\n";
      if (auto E = SQLWhereClauseEmitter(OS).run(WhereClause))
        return std::move(E);
    }

    auto OrderedBy = QueryRecord->getValueAsListOfStrings("OrderedBy");
    if (OrderedBy.size()) {
      OS << "\n";
      OS << "ORDER BY ";
      ListSeparator LS;
      for (auto FieldOrTag : OrderedBy) {
        auto FieldName = FieldOrTag;
        if (FieldOrTag.startswith("$")) {
          // It's a tag
          auto TagName = FieldOrTag.drop_front(1);
          auto I = FieldTagMap.find(TagName);
          if (I == FieldTagMap.end())
            return createTGStringError(QueryRecord->getLoc()[0], "Unrecognized "
                                       "tag \"" + TagName + "\"");
          FieldName = I->second;
        }
        OS << LS << FieldName;
      }
    }

    OS << ";\n\n";
  }

  return Error::success();
}
