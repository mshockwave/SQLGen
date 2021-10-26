#include "Error.h"

char llvm::SMLocError::ID = 0;

llvm::Error llvm::createTGStringError(SMLoc Loc, const Twine &S) {
  return make_error<SMLocError>(Loc, inconvertibleErrorCode(), S);
}
