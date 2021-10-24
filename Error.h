#include "llvm/ADT/Twine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/SMLoc.h"

namespace llvm {
class SMLocError : public ErrorInfo<SMLocError, StringError> {
  using ErrorInfo<SMLocError, StringError>::ErrorInfo;

public:
  static char ID;
  SMLoc Loc;

  SMLocError(SMLoc Loc, std::error_code EC, const Twine &S = Twine())
    : ErrorInfo(EC, S), Loc(Loc) {}
};

Error createTGStringError(SMLoc Loc, const Twine &S);
} // end namespace llvm
