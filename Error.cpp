#include "Error.h"

using namespace llvm;

char SMLocError::ID = 0;

Error llvm::createTGStringError(SMLoc Loc, const Twine &S) {
  return make_error<SMLocError>(Loc, inconvertibleErrorCode(), S);
}
