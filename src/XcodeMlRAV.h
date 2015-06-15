#ifndef XCODEMLRAV_H
#define XCODEMLRAV_H

#include "clang/AST/AST.h"
#include "clang/Tooling/CommonOptionsParser.h"

extern llvm::cl::OptionCategory C2XcodeMLCategory;

// Bi-directional Bridging interface for RecursiveASTVisitor (abstract class)
class RAVBidirBridge {
protected:
    RAVBidirBridge *const otherside;
public:
    RAVBidirBridge() : otherside(nullptr) {};
    RAVBidirBridge(const RAVBidirBridge &) = delete;
    RAVBidirBridge(RAVBidirBridge &&) = delete;
    RAVBidirBridge &operator =(const RAVBidirBridge &) = delete;
    RAVBidirBridge &operator =(RAVBidirBridge &&) = delete;

    explicit RAVBidirBridge(RAVBidirBridge *B) : otherside(B) {};

    virtual bool BridgeStmt(clang::Stmt *S) = 0;
    virtual bool BridgeType(clang::QualType T) = 0;
    virtual bool BridgeTypeLoc(clang::TypeLoc TL) = 0;
    virtual bool BridgeAttr(clang::Attr *At) = 0;
    virtual bool BridgeDecl(clang::Decl *D) = 0;
    virtual bool
      BridgeNestedNameSpecifier(clang::NestedNameSpecifier *NNS) = 0;
    virtual bool
      BridgeNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NNS) = 0;
    virtual bool
      BridgeDeclarationNameInfo(clang::DeclarationNameInfo NameInfo) = 0;
    virtual bool BridgeTemplateName(clang::TemplateName Template) = 0;
    virtual bool
      BridgeTemplateArgument(const clang::TemplateArgument &Arg) = 0;
    virtual bool
      BridgeTemplateArgumentLoc(const clang::TemplateArgumentLoc &ArgLoc) = 0;
    //virtual bool BridgeTemplateArguments(const TemplateArgument *Args,
    //                                     unsigned NumArgs) = 0;
    virtual bool
      BridgeConstructorInitializer(clang::CXXCtorInitializer *Init) = 0;
    //virtual bool BridgeLambdaCapture(LambdaExpr *LE,
    //                                 const LambdaCapture *C) = 0;
    //virtual bool BridgeLambdaBody(LambdaExpr *LE) = 0;

    virtual const char *getVisitorName() const = 0;
};

class XcodeMlRAVpool : public RAVBidirBridge {
    friend class RAVpoolSizeChecker;
private:
    // memory pool for XcodeMlRAV: hide implemantation completely
    char RAVpool[sizeof(RAVBidirBridge)]; //enough?
public:
    XcodeMlRAVpool() = delete;
    XcodeMlRAVpool(const XcodeMlRAVpool &) = delete;
    XcodeMlRAVpool(XcodeMlRAVpool &&) = delete;
    XcodeMlRAVpool & operator =(const XcodeMlRAVpool &) = delete;
    XcodeMlRAVpool & operator =(XcodeMlRAVpool &&) = delete;
    explicit XcodeMlRAVpool(RAVBidirBridge *);
};

#endif /* !XCODEMLRAV_H */
///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
