#ifndef SQLGEN_ERROR_H
#define SQLGEN_ERROR_H
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/SMLoc.h"

namespace llvm {
struct SMLocError : public ErrorInfo<SMLocError, StringError> {
  static char ID;
  SMLoc Loc;

  SMLocError(SMLoc Loc, std::error_code EC, const Twine &S = Twine())
    : ErrorInfo(EC, S), Loc(Loc) {}
};

Error createTGStringError(SMLoc Loc, const Twine &S);
} // end namespace llvm
#endif
