#include "XcodeMlVisitorBase.h"
#include "TypeTableVisitor.h"

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

const char *
TypeTableVisitor::NameForStmt(Stmt *S) {
  (void)S;
  return ""; // do not create a new child
}

const char *
TypeTableVisitor::NameForDecl(Decl *D) {
  if (D->getKind() == Decl::TranslationUnit) {
    if (OptDisableTypeTable) {
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
