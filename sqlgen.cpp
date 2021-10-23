#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Record.h"

using namespace llvm;

namespace {
enum ActionType {
  PrintRecords,
  PrintDetailedRecords
};
} // end anonymous namespace

static cl::opt<ActionType>
  Action(cl::desc("Actions to perform"),
         cl::values(
           clEnumValN(PrintRecords, "print-records", ""),
           clEnumValN(PrintDetailedRecords, "print-detailed-records", "")
         ),
         cl::init(PrintRecords));

bool SQLGenMain(raw_ostream &OS, RecordKeeper &Records) {
  switch (Action) {
  case PrintRecords:
    OS << Records;
    break;
  default:
    return true;
  }

  return false;
}

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv);
  return llvm::TableGenMain(argv[0], &SQLGenMain);
}
