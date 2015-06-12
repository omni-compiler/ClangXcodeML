#include "XcodeMlVisitorBase.h"
#include "SymbolsVisitor.h"

using namespace llvm;

static cl::opt<bool>
OptTraceSymbols("trace-symbols",
                cl::desc("emit traces on <globalSymbols>, <symbols>"),
                cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableSymbols("disable-symbols",
                  cl::desc("disable <globalSymbols>, <symbols>"),
                  cl::cat(C2XcodeMLCategory));

const char *
SymbolsVisitor::getVisitorName() const {
  return OptTraceSymbols ? "Symbols" : nullptr;
}

const char *
SymbolsVisitor::NameForDecl(Decl *D) {
  if (D->getKind() == Decl::TranslationUnit) {
    if (OptDisableSymbols) {
      return nullptr; // stop traverse
    } else {
      return ""; // no need to create a child
    }
  }
  return "TraverseDecl";
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
