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

  for (const auto *Class : Classes) {
    StringRef PrimaryKey;
    TableMembers.clear();
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
          PrimaryKey = Name;
      }

      SQLType SQLTy = SQLType::Unknown;
      switch (RV.getType()->getRecTyKind()) {
      case RecTy::IntRecTyKind:
      case RecTy::RecordRecTyKind:
        // TODO: Full support for RecordRecTy
        SQLTy = SQLType::Integer;
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

    OS << "CREATE TABLE " << Class->getName() << " (\n";
    for (const auto &Member : TableMembers)
      OS.indent(4) << Member.second << " " << Member.first <<",\n";
    if (PrimaryKey.size())
      OS.indent(4) << "PRIMARY KEY (" << PrimaryKey << ")\n";
    OS << ");\n";
  }

  return Error::success();
}
