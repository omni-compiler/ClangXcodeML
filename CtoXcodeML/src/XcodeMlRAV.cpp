#include "XcodeMlRAV.h"

#include "clang/Driver/Options.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <type_traits>

using namespace clang;
using namespace llvm;

static cl::opt<bool> OptTraceRAV("trace-rav",
    llvm::cl::desc("trace Recursive AST Visitor"),
    llvm::cl::cat(C2XcodeMLCategory));

// implement my RecursiveASTVisitor (which uses CRTP)
class XcodeMlRAV : public clang::RecursiveASTVisitor<XcodeMlRAV>,
                   public RAVBidirBridge {
private:
  typedef RecursiveASTVisitor<XcodeMlRAV> RAV;

public:
  XcodeMlRAV() = delete;
  XcodeMlRAV(const XcodeMlRAV &) = delete;
  XcodeMlRAV(XcodeMlRAV &&) = delete;
  XcodeMlRAV &operator=(const XcodeMlRAV &) = delete;
  XcodeMlRAV &operator=(XcodeMlRAV &&) = delete;

  explicit XcodeMlRAV(RAVBidirBridge *otherside) : RAVBidirBridge(otherside){};

  bool
  shouldUseDataRecursionFor(Stmt *S) const {
    (void)S;
    return false;
  }

#define DISPATCHER(NAME, TYPE)                                                \
  bool Traverse##NAME(TYPE S) {                                               \
    const char *VN = otherside->getVisitorName();                             \
    if (OptTraceRAV && VN) {                                                  \
      errs() << VN << "::       Traverse" #NAME "\n";                         \
    }                                                                         \
    return otherside->Bridge##NAME(S);                                        \
  }                                                                           \
  bool Bridge##NAME(TYPE S) override {                                        \
    const char *VN = otherside->getVisitorName();                             \
    if (OptTraceRAV && VN) {                                                  \
      errs() << VN << "::BridgeTraverse" #NAME "\n";                          \
    }                                                                         \
    return static_cast<RAV *>(this)->Traverse##NAME(S);                       \
  }

  DISPATCHER(Stmt, Stmt *);
  DISPATCHER(Type, QualType);
  DISPATCHER(TypeLoc, TypeLoc);
  DISPATCHER(Attr, Attr *);
  DISPATCHER(Decl, Decl *);
  DISPATCHER(NestedNameSpecifier, NestedNameSpecifier *);
  DISPATCHER(NestedNameSpecifierLoc, NestedNameSpecifierLoc);
  DISPATCHER(DeclarationNameInfo, DeclarationNameInfo);
  DISPATCHER(TemplateName, TemplateName);
  DISPATCHER(TemplateArgument, const TemplateArgument &);
  DISPATCHER(TemplateArgumentLoc, const TemplateArgumentLoc &);
  DISPATCHER(ConstructorInitializer, CXXCtorInitializer *);

  const char *
  getVisitorName() const override {
    return "RAV";
  }
};

class RAVpoolSizeChecker {
  static_assert(sizeof(XcodeMlRAV) <= sizeof(XcodeMlRAVpool::RAVpool),
      "XcodeMlRAVpool::RAVpool is too small");
};

XcodeMlRAVpool::XcodeMlRAVpool(RAVBidirBridge *bridge)
    : RAVBidirBridge(new (RAVpool) XcodeMlRAV(bridge)){};

cl::OptionCategory C2XcodeMLCategory("CtoXcodeML options");
