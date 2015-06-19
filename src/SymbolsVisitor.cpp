#include "XcodeMlVisitorBase.h"
#include "SymbolsVisitor.h"

using namespace clang;
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

bool
SymbolsVisitor::PreVisitStmt(Stmt *S) {
  (void)S;
  return false; // do not traverse children
}

bool
SymbolsVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return false;
  }
  if (D->getKind() == Decl::TranslationUnit) {
    if (OptDisableSymbols) {
      return false; // stop traverse
    } else {
      return true; // no need to create a child
    }
  }
  newChild("TraverseDecl");
  return true;
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
