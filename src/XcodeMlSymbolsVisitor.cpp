#include "XcodeMlVisitorBase.h"
#include "XcodeMlSymbolsVisitor.h"

using namespace llvm;

static cl::opt<bool>
OptTraceXSV("trace-xsv",
            cl::desc("emit traces on XcodeMlSymbolVisitor"),
            cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXSV("disable-xsv",
              cl::desc("disable XcodeMlSymbolsVisitor"),
              cl::cat(C2XcodeMLCategory));

const char *
XcodeMlSymbolsVisitor::getVisitorName() const {
  return OptTraceXSV ? "XSV" : nullptr;
}

const char *
XcodeMlSymbolsVisitor::NameForDecl(Decl *D) const {
  if (D->getKind() == Decl::TranslationUnit) {
    if (OptDisableXSV) {
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
