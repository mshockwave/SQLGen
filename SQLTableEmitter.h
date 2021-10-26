#ifndef SQLGEN_SQLTABLEEMITTER_H
#define SQLGEN_SQLTABLEEMITTER_H
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Error.h"
#include "llvm/TableGen/Record.h"

namespace llvm {
class SQLTableEmitter {
  raw_ostream &OS;
  // Map from a class (record) to its primary key member.
  DenseMap<const Record *, StringRef> PrimaryKeys;

public:
  explicit
  SQLTableEmitter(raw_ostream &OS) : OS(OS) {}

  Error run(ArrayRef<const Record *> Classes);

  Optional<StringRef> getPrimaryKey(const Record *ClassRecord) const {
    auto I = PrimaryKeys.find(ClassRecord);
    if (I != PrimaryKeys.end())
      return I->second;
    return {};
  }
};
} // end namespace llvm
#endif
