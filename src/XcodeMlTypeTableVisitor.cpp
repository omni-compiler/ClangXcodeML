#include "XcodeMlVisitorBase.h"
#include "XcodeMlTypeTableVisitor.h"

using namespace llvm;

static cl::opt<bool>
OptTraceXTTV("trace-xttv",
             cl::desc("emit traces on XcodeMlTypeTableVisitor"),
             cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXTTV("disable-xttv",
               cl::desc("disable XcodeMlTypeTableVisitor"),
               cl::cat(C2XcodeMLCategory));

const char *
XcodeMlTypeTableVisitor::getVisitorName() const {
  return OptTraceXTTV ? "XTTV" : nullptr;
}

const char *
XcodeMlTypeTableVisitor::NameForStmt(Stmt *S) const {
  return ""; // do not create a new child
}

const char *
XcodeMlTypeTableVisitor::NameForDecl(Decl *D) const {
  if (D->getKind() == Decl::TranslationUnit) {
    if (OptDisableXTTV) {
      return nullptr; // stop traverse
    } else {
      return ""; // no need to create a child
    }
  }
  return ""; // do not create a new child
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
