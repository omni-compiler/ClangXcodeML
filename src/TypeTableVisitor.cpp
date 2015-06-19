#include "XcodeMlVisitorBase.h"
#include "TypeTableVisitor.h"

using namespace clang;
using namespace llvm;

static cl::opt<bool>
OptTraceTypeTable("trace-typeTable",
                  cl::desc("emit traces on <typeTable>"),
                  cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableTypeTable("disable-typeTable",
                    cl::desc("disable <typeTable>"),
                    cl::cat(C2XcodeMLCategory));

const char *
TypeTableVisitor::getVisitorName() const {
  return OptTraceTypeTable ? "TypeTable" : nullptr;
}

bool
TypeTableVisitor::PreVisitStmt(Stmt *S) {
  (void)S;
  return true; // do not create a new child
}

bool
TypeTableVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return false;
  }
  if (D->getKind() == Decl::TranslationUnit) {
    if (OptDisableTypeTable) {
      return false; // stop traverse
    } else {
      return true; // no need to create a child
    }
  }
  return true; // do not create a new child
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
