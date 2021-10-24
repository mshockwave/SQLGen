#ifndef SQLGEN_SQLTABLEEMITTER_H
#define SQLGEN_SQLTABLEEMITTER_H
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Error.h"
#include "llvm/TableGen/Record.h"

namespace llvm {
class SQLTableEmitter {
  raw_ostream &OS;
  // Map from a class (record) to its primary key member.
  DenseMap<const Record *, StringRef> PrimaryKeys;

public:
  SQLTableEmitter(raw_ostream &OS) : OS(OS) {}

  Error run(ArrayRef<const Record *> Classes);
};
} // end namespace llvm
#endif
