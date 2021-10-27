#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

#include "SQLInsertEmitter.h"
#include "SQLQueryEmitter.h"
#include "SQLTableEmitter.h"

using namespace llvm;

Error emitSQL(raw_ostream &OS, RecordKeeper &Records) {

  if (const auto *TableClass = Records.getClass("Table")) {
    // Generate SQL tables
    const auto &Classes = Records.getClasses();
    SmallVector<const Record *, 4> TableClasses;
    for (const auto &P : Classes) {
      if (P.second->isSubClassOf(TableClass))
        TableClasses.push_back(P.second.get());
    }
    SQLTableEmitter TableEmitter(OS);
    if (auto E = TableEmitter.run(TableClasses))
      return std::move(E);

    auto getPrimaryKey = [&](const Record *Class) -> Optional<StringRef> {
      return TableEmitter.getPrimaryKey(Class);
    };

    // Generate SQL rows
    auto SQLRows = Records.getAllDerivedDefinitions("Table");
    // RecordKeeper::getAllDerivedDefinitions will only
    // return concrete records, so we don't need to filter out
    // class `Record` instances.
    if (auto E = SQLInsertEmitter(OS, getPrimaryKey).run(SQLRows))
      return std::move(E);
  }

  if (Records.getClass("Query")) {
    auto SQLQueries = Records.getAllDerivedDefinitions("Query");
    if (auto E = SQLQueryEmitter(OS).run(SQLQueries))
      return std::move(E);
  }

  return Error::success();
}
