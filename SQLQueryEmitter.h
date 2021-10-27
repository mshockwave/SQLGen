#ifndef SQLGEN_SQLQUERYEMITTER_H
#define SQLGEN_SQLQUERYEMITTER_H
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace llvm {
class SQLQueryEmitter {
  raw_ostream &OS;

public:
  explicit
  SQLQueryEmitter(raw_ostream &OS) : OS(OS) {}

  Error run(ArrayRef<const Record *> Queries);
};
} // end namespace llvm
#endif
