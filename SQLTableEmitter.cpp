#include "llvm/ADT/StringExtras.h"

#include "Error.h"
#include "SQLTableEmitter.h"

using namespace llvm;

namespace {
enum class SQLType {
  Unknown,
  Integer,
  VarChar
};
} // end anonymous namespace

namespace llvm {
raw_ostream &operator<<(raw_ostream &OS, const SQLType &Ty) {
  switch (Ty) {
  case SQLType::Integer:
    OS << "int";
    break;
  case SQLType::VarChar:
    OS << "varchar(255)";
    break;
  default:
    break;
  }
  return OS;
}
} // end namespace llvm

Error SQLTableEmitter::run(ArrayRef<const Record *> Classes) {
  // {SQL data type, member name}
  SmallVector<std::pair<SQLType, StringRef>, 4> TableMembers;
  // {foreign key member, referee class, referee primary key}
  SmallVector<std::tuple<StringRef, const Record *, StringRef>, 4>
    ForeignKeys;

  for (const auto *Class : Classes) {
    StringRef CurPrimaryKey;
    TableMembers.clear();
    ForeignKeys.clear();
    for (const auto &RV : Class->getValues()) {
      // We only care about member variables.
      if (RV.isTemplateArg())
        continue;

      auto Name = RV.getName();
      // The PrimaryKey member is not directly used.
      if (Name == "PrimaryKey")
        continue;

      if (auto *VI = dyn_cast<VarInit>(RV.getValue())) {
        if (VI->getName() == "PrimaryKey")
          CurPrimaryKey = Name;
      }

      SQLType SQLTy = SQLType::Unknown;
      const auto *RVT = RV.getType();
      switch (RVT->getRecTyKind()) {
      case RecTy::IntRecTyKind:
      case RecTy::RecordRecTyKind:
        SQLTy = SQLType::Integer;
        if (const auto *RRT = dyn_cast<RecordRecTy>(RVT)) {
          // Implement members with RecordRecTy with foreign keys.
          bool FoundFK = false;
          for (const auto *C : RRT->getClasses())
            if (PrimaryKeys.count(C)) {
              ForeignKeys.push_back({Name, C, PrimaryKeys[C]});
              FoundFK = true;
              break;
            }

          if (!FoundFK)
            return createTGStringError(RV.getLoc(), "Cannot locate primary key "
                                       "of the referred table");
        }
        break;
      case RecTy::StringRecTyKind:
        SQLTy = SQLType::VarChar;
        break;
      default:
        return createTGStringError(RV.getLoc(),
                                   "Unsupported table member type");
      }
      TableMembers.push_back({SQLTy, Name});
    }

    // Write down the primary key of this table.
    if (CurPrimaryKey.size())
      PrimaryKeys.insert({Class, CurPrimaryKey});

    ListSeparator LS(",\n");
    OS << "CREATE TABLE " << Class->getName() << " (\n";
    for (const auto &Member : TableMembers)
      (OS << LS).indent(4) << Member.second << " " << Member.first;
    if (CurPrimaryKey.size())
      (OS << LS).indent(4) << "PRIMARY KEY (" << CurPrimaryKey << ")";
    for (const auto &FK : ForeignKeys)
      (OS << LS).indent(4) << "FOREIGN KEY (" << std::get<0>(FK) << ") "
                           << "REFERENCES " << std::get<1>(FK)->getName()
                           << "(" << std::get<2>(FK) << ")";
    OS << "\n);\n\n";
  }

  return Error::success();
}
