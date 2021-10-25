#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

#include "SQLInsertEmitter.h"
#include "SQLTableEmitter.h"

using namespace llvm;

Error emitSQL(raw_ostream &OS, RecordKeeper &Records) {

  // Generate SQL tables
  const auto &Classes = Records.getClasses();
  SmallVector<const Record *, 4> TableClasses;
  for (const auto &P : Classes) {
    if (P.second->isSubClassOf("Table"))
      TableClasses.push_back(P.second.get());
  }
  SQLTableEmitter TableEmitter(OS);
  if (auto E = TableEmitter.run(TableClasses))
    return std::move(E);

  auto getPrimaryKey = [&](const Record *Class) -> Optional<StringRef> {
    return TableEmitter.getPrimaryKey(Class);
  };

  // RecordKeeper::getAllDerivedDefinitions will only
  // return concrete records, so we don't need to filter out
  // class `Record` instances.
  auto SQLRows = Records.getAllDerivedDefinitions("Table");
  if (auto E = SQLInsertEmitter(OS, getPrimaryKey).run(SQLRows))
    return std::move(E);

  return Error::success();
}
