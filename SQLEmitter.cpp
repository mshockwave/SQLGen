#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

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
  if (auto E = SQLTableEmitter(OS).run(TableClasses))
    return std::move(E);

  return Error::success();
}
