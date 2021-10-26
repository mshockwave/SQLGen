#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringMap.h"
#include "Error.h"
#include "SQLQueryEmitter.h"

using namespace llvm;

namespace {
class SQLWhereClauseEmitter {
  raw_ostream &OS;

public:
  SQLWhereClauseEmitter(raw_ostream &OS) : OS(OS) {}

  Error run(const DagInit *WhereClause);
};
} // end anonymous namespace

Error SQLWhereClauseEmitter::run(const DagInit *WhereClause) {
  // TODO: Build an operator tree and visit it in an infix fashion.
  return Error::success();
}

Error SQLQueryEmitter::run(ArrayRef<const Record *> Queries,
                           ArrayRef<const Record *> Operators) {
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
