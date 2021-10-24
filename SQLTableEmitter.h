#ifndef SQLGEN_SQLTABLEEMITTER_H
#define SQLGEN_SQLTABLEEMITTER_H
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Error.h"
#include "llvm/TableGen/Record.h"

namespace llvm {
class SQLTableEmitter {
  raw_ostream &OS;

public:
  SQLTableEmitter(raw_ostream &OS) : OS(OS) {}

  Error run(ArrayRef<const Record *> Classes);
};
} // end namespace llvm
#endif
