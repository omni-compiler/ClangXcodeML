#include "XMLRAV.h"

#include "clang/Driver/Options.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <type_traits>

using namespace clang;
using namespace llvm;

static cl::opt<bool> OptTraceRAV("trace-rav",
    llvm::cl::desc("trace Recursive AST Visitor"),
    llvm::cl::cat(CXX2XMLCategory));

// implement my RecursiveASTVisitor (which uses CRTP)
class XMLRAV : public clang::RecursiveASTVisitor<XMLRAV>,
               public RAVBidirBridge {
private:
  typedef RecursiveASTVisitor<XMLRAV> RAV;

public:
  XMLRAV() = delete;
  XMLRAV(const XMLRAV &) = delete;
  XMLRAV(XMLRAV &&) = delete;
  XMLRAV &operator=(const XMLRAV &) = delete;
  XMLRAV &operator=(XMLRAV &&) = delete;

  explicit XMLRAV(RAVBidirBridge *otherside) : RAVBidirBridge(otherside){};

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
  static_assert(sizeof(XMLRAV) <= sizeof(XMLRAVpool::RAVpool),
      "XMLRAVpool::RAVpool is too small");
};

XMLRAVpool::XMLRAVpool(RAVBidirBridge *bridge)
    : RAVBidirBridge(new (RAVpool) XMLRAV(bridge)){};


