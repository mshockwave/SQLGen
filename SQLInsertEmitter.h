#ifndef SQLGEN_SQLINSERTEMITTER_H
#define SQLGEN_SQLINSERTEMITTER_H
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Record.h"

namespace llvm {
class SQLInsertEmitter {
  raw_ostream &OS;

  using PKCallbackTy = Optional<StringRef>(const Record *);
  function_ref<PKCallbackTy> GetPrimaryKeyCB;

  // Inserted SQL row -> its primary key.
  DenseMap<const Record *, unsigned> InsertedRows;
  // SQL Table's TG class -> number of rows.
  DenseMap<const Record *, unsigned> TableSizes;

  Error insertRowImpl(const Record *Row);

public:
  SQLInsertEmitter(raw_ostream &OS, function_ref<PKCallbackTy> CB)
    : OS(OS), GetPrimaryKeyCB(CB) {}

  Error run(ArrayRef<const Record *> Rows);
};
} // end namespace llvm
#endif
