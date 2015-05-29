#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

// interface (abstract class)
class XcodeMlVisitorBidirectionalBridge {
private:
    typedef XcodeMlVisitorBidirectionalBridge MYSELF;
protected:
    XcodeMlVisitorBidirectionalBridge *const otherside;
public:
    XcodeMlVisitorBidirectionalBridge() = delete;
    XcodeMlVisitorBidirectionalBridge(const MYSELF &) = delete;
    XcodeMlVisitorBidirectionalBridge(MYSELF &&) = delete;
    MYSELF &operator =(const MYSELF &) = delete;
    MYSELF &operator =(MYSELF &&) = delete;

    explicit XcodeMlVisitorBidirectionalBridge(MYSELF *B) : otherside(B) {};

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT)                                             \
    virtual bool XcodeMlTraverse##CLASS(CLASS *S) = 0;
#include "clang/AST/StmtNodes.inc"

#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                               \
    virtual bool XcodeMlTraverse##CLASS##Type(CLASS##Type *T) = 0;
#include "clang/AST/TypeNodes.def"

    virtual bool XcodeMlTraverseDecl(Decl *D) = 0;
#define ABSTRACT_DECL(DECL)
#define DECL(CLASS, BASE)                                               \
    virtual bool XcodeMlTraverse##CLASS##Decl(CLASS##Decl *D) = 0;
#include "clang/AST/DeclNodes.inc"

    virtual const char *getVisitorName() const = 0;
};

//
// XcodeMlRAV: use RecursiveASTVisitor
//
class XcodeMlRAV : public RecursiveASTVisitor<XcodeMlRAV>,
                   public XcodeMlVisitorBidirectionalBridge
{
public:
    XcodeMlRAV() = delete;
    XcodeMlRAV(const XcodeMlRAV&) = delete;
    XcodeMlRAV(XcodeMlRAV&&) = delete;
    XcodeMlRAV& operator =(const XcodeMlRAV&) = delete;
    XcodeMlRAV& operator =(XcodeMlRAV&&) = delete;

    explicit XcodeMlRAV(XcodeMlVisitorBidirectionalBridge *B)
        : XcodeMlVisitorBidirectionalBridge(B) {};

    typedef RecursiveASTVisitor<XcodeMlRAV> RAV;
    bool shouldVisitImplicitCode() const { return true; }

    // avoid data-recursion (force traversing with hardware stack)
    bool shouldUseDataRecursionFor(Stmt *S) const { return false; }

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT)                                     \
    bool Traverse##CLASS(CLASS *S);                             \
    bool XcodeMlTraverse##CLASS(CLASS *S) override;
#include "clang/AST/StmtNodes.inc"

#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                       \
    bool Traverse##CLASS##Type(CLASS##Type *T);                 \
    bool XcodeMlTraverse##CLASS##Type(CLASS##Type *T) override;
#include "clang/AST/TypeNodes.def"

    bool TraverseDecl(Decl *D);
    bool XcodeMlTraverseDecl(Decl *D) override;

#define ABSTRACT_DECL(DECL)
#define DECL(CLASS, BASE)                                       \
    bool Traverse##CLASS##Decl(CLASS##Decl *D);                 \
    bool XcodeMlTraverse##CLASS##Decl(CLASS##Decl *D) override;
#include "clang/AST/DeclNodes.inc"
    const char *getVisitorName() const override { return "RAV"; }
};

// some members & methods of XcodeMlVisitorBase do not need the info
// of deriving type <Derived>:
// those members & methods are separated from XcodeMlvisitorBase.
// (those methods can be written in the source of XcodeMlVisitorBase.cpp)
class XcodeMlVisitorBaseImpl : public XcodeMlVisitorBidirectionalBridge {
private:
    XcodeMlRAV RAV;
protected:
    const xmlNodePtr rootNode;    // the current root node.
    xmlNodePtr curNode;           // a candidate of the new chlid.
    bool isLocationAlreadySet;
    const ASTContext &astContext;
public:
    XcodeMlVisitorBaseImpl() = delete;
    XcodeMlVisitorBaseImpl(const XcodeMlVisitorBaseImpl&) = delete;
    XcodeMlVisitorBaseImpl(XcodeMlVisitorBaseImpl&&) = delete;
    XcodeMlVisitorBaseImpl& operator =(const XcodeMlVisitorBaseImpl&) = delete;
    XcodeMlVisitorBaseImpl& operator =(XcodeMlVisitorBaseImpl&&) = delete;

    explicit XcodeMlVisitorBaseImpl(const ASTContext &CXT, xmlNodePtr N,
                                    const char *Name = nullptr);
    explicit XcodeMlVisitorBaseImpl(const XcodeMlVisitorBaseImpl *p,
                                    const char *Name = nullptr)
        : XcodeMlVisitorBaseImpl(p->astContext, p->curNode, Name) {}

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
    using XcodeMlVisitorBaseImpl::XcodeMlVisitorBaseImpl;

    Derived &getDerived() { return static_cast<Derived &>(*this); }

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT)                                     \
    const char *NameFor##CLASS(CLASS *S) const {                \
        return "Traverse" #CLASS;                               \
    }                                                           \
    bool PostVisit##CLASS(CLASS *S) {                           \
        xmlAddChild(rootNode, curNode);                         \
        return true;                                            \
    }                                                           \
    bool XcodeMlTraverse##CLASS(CLASS *S) override {            \
        Derived V(this, NameFor##CLASS(S));                     \
        newComment("XcodeMlTraverse" #CLASS);                   \
        if (!V.otherside->XcodeMlTraverse##CLASS(S)) {          \
            return false;                                       \
        }                                                       \
        return V.getDerived().PostVisit##CLASS(S);              \
    }
#include "clang/AST/StmtNodes.inc"

#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                       \
    const char *NameFor##CLASS##Type(CLASS##Type *T) const {    \
        return "Traverse" #CLASS "Type";                        \
    }                                                           \
    bool PostVisit##CLASS##Type(CLASS##Type *T) {               \
        xmlAddChild(rootNode, curNode);                         \
        return true;                                            \
    }                                                           \
    bool XcodeMlTraverse##CLASS##Type(CLASS##Type *T) {         \
        Derived V(this, NameFor##CLASS##Type(T));               \
        newComment("XcodeMlTraverse" #CLASS "Type");            \
        if (!V.otherside->XcodeMlTraverse##CLASS##Type(T)) {    \
            return false;                                       \
        }                                                       \
        return V.getDerived().PostVisit##CLASS##Type(T);        \
    }
#include "clang/AST/TypeNodes.def"

    const char *NameForDecl(Decl *D) const {
        return "TraverseDecl";
    }
    bool XcodeMlTraverseDecl(Decl *D) {
        Derived V(this, "TraverseDecl");
        newComment("XcodeMlTraverseDecl");
        return V.otherside->XcodeMlTraverseDecl(D);
    }

#define ABSTRACT_DECL(DECL)
#define DECL(CLASS, BASE)                                       \
    const char *NameFor##CLASS##Decl(CLASS##Decl *D) const {    \
        return "Traverse" #CLASS "Decl";                        \
    }                                                           \
    bool PostVisit##CLASS##Decl(CLASS##Decl *D) {               \
        setLocation(D);                                         \
        xmlAddChild(rootNode, curNode);                         \
        return true;                                            \
    }                                                           \
    bool XcodeMlTraverse##CLASS##Decl(CLASS##Decl *D) {         \
        newComment("XcodeMlTraverseDecl" #CLASS "Decl");        \
        setName(NameFor##CLASS##Decl(D));                       \
        if (!otherside->XcodeMlTraverse##CLASS##Decl(D)) {      \
            return false;                                       \
        }                                                       \
        return getDerived().PostVisit##CLASS##Decl(D);          \
    }
#include "clang/AST/DeclNodes.inc"
};

extern cl::OptionCategory C2XcodeMLCategory;

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
