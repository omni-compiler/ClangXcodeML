#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/AST/AST.h"
#include <libxml/tree.h>

using namespace clang;

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

    virtual bool BridgeStmt(Stmt *S) = 0;
    virtual bool BridgeType(QualType T) = 0;
    virtual bool BridgeTypeLoc(TypeLoc TL) = 0;
    virtual bool BridgeAttr(Attr *At) = 0;
    virtual bool BridgeDecl(Decl *D) = 0;
    virtual bool BridgeNestedNameSpecifier(NestedNameSpecifier *NNS) = 0;
    virtual bool BridgeNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS) = 0;
    virtual bool BridgeDeclarationNameInfo(DeclarationNameInfo NameInfo) = 0;
    virtual bool BridgeTemplateName(TemplateName Template) = 0;
    virtual bool BridgeTemplateArgument(const TemplateArgument &Arg) = 0;
    virtual bool
    BridgeTemplateArgumentLoc(const TemplateArgumentLoc &ArgLoc) = 0;
    //virtual bool BridgeTemplateArguments(const TemplateArgument *Args,
    //                                     unsigned NumArgs) = 0;
    virtual bool BridgeConstructorInitializer(CXXCtorInitializer *Init) = 0;
    //virtual bool BridgeLambdaCapture(LambdaExpr *LE,
    //                                 const LambdaCapture *C) = 0;
    //virtual bool BridgeLambdaBody(LambdaExpr *LE) = 0;

    virtual const char *getVisitorName() const = 0;
};

// some members & methods of XcodeMlVisitorBase do not need the info
// of deriving type <Derived>:
// those members & methods are separated from XcodeMlvisitorBase.
// (those methods can be written in the source of XcodeMlVisitorBase.cpp)
class XcodeMlVisitorBaseImpl : public RAVBidirBridge {
    friend class RAVpoolSizeChecker;
private:
    // memory pool for XcodeMlRAV: hide implemantation completely
    char RAVpool[sizeof(RAVBidirBridge)]; //enough?
protected:
    const ASTContext &astContext;
    const xmlNodePtr rootNode;    // the current root node.
    xmlNodePtr curNode;           // a candidate of the new chlid.
    bool isLocationAlreadySet;
public:
    XcodeMlVisitorBaseImpl() = delete;
    XcodeMlVisitorBaseImpl(const XcodeMlVisitorBaseImpl&) = delete;
    XcodeMlVisitorBaseImpl(XcodeMlVisitorBaseImpl&&) = delete;
    XcodeMlVisitorBaseImpl& operator =(const XcodeMlVisitorBaseImpl&) = delete;
    XcodeMlVisitorBaseImpl& operator =(XcodeMlVisitorBaseImpl&&) = delete;

    explicit XcodeMlVisitorBaseImpl(const ASTContext &CXT,
                                    xmlNodePtr RootNode,
                                    xmlNodePtr CurNode);

    void setName(const char *Name);
    void newProp(const char *Name, int Val, xmlNodePtr N = nullptr);
    void newProp(const char *Name, const char *Val, xmlNodePtr N = nullptr);
    void newComment(const xmlChar *str, xmlNodePtr RN = nullptr);
    void newComment(const char *str, xmlNodePtr RN = nullptr);
    void setLocation(SourceLocation Loc, xmlNodePtr N = nullptr);
    void setLocation(const Decl *D, xmlNodePtr N = nullptr);
    void setLocation(const Expr *E, xmlNodePtr N = nullptr);
};

// Main class: XcodeMlVisitorBase<Derived>
// this is CRTP (Curiously Recurring Template Pattern)
template <class Derived>
class XcodeMlVisitorBase : public XcodeMlVisitorBaseImpl {
public:
    XcodeMlVisitorBase() = delete;
    XcodeMlVisitorBase(const XcodeMlVisitorBase&) = delete;
    XcodeMlVisitorBase(XcodeMlVisitorBase&&) = delete;
    XcodeMlVisitorBase& operator =(const XcodeMlVisitorBase&) = delete;
    XcodeMlVisitorBase& operator =(XcodeMlVisitorBase&&) = delete;

    explicit XcodeMlVisitorBase(const ASTContext &CXT, xmlNodePtr RootNode,
                                const char *ChildName)
        : XcodeMlVisitorBaseImpl(CXT, RootNode,
                                 (ChildName
                                  ? xmlNewChild(RootNode, nullptr,
                                                BAD_CAST ChildName, nullptr)
                                  : RootNode)) {};
    explicit XcodeMlVisitorBase(const XcodeMlVisitorBase *p)
        : XcodeMlVisitorBaseImpl(p->astContext, p->curNode, p->curNode) {};

    Derived &getDerived() { return *static_cast<Derived *>(this); }

#define DISPATCHER(NAME, TYPE)                                          \
    const char *NameFor##NAME(TYPE S) const {                           \
        return "Traverse" #NAME;                                        \
    }                                                                   \
    const char *ContentsFor##NAME(TYPE S) const {                       \
        return nullptr;                                                 \
    }                                                                   \
    bool PreVisit##NAME(TYPE S) {                                       \
        const char *Name = getDerived().NameFor##NAME(S);               \
        if (!Name) {                                                    \
            return false;                                               \
        }                                                               \
        const char *Contents = getDerived().ContentsFor##NAME(S);       \
        curNode = xmlNewChild(rootNode, nullptr, BAD_CAST Name,         \
                              BAD_CAST Contents);                       \
        return true;                                                    \
    }                                                                   \
    bool PostVisit##NAME(TYPE S) {                                      \
        return true;							\
    }                                                                   \
    bool Bridge##NAME(TYPE S) override {                                \
        Derived V(this);                                                \
        newComment("Bridge" #NAME);                                     \
        return (V.getDerived().PreVisit##NAME(S)                        \
                && V.otherside->Bridge##NAME(S)                         \
                && V.getDerived().PostVisit##NAME(S));                  \
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
#undef DISPATCHER
};

extern llvm::cl::OptionCategory C2XcodeMLCategory;

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
