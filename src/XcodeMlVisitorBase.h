#ifndef XCODEMLVISITORBASE_H
#define XCODEMLVISITORBASE_H

#include "XcodeMlRAV.h"
#include "llvm/ADT/SmallVector.h"
#include "clang/AST/Mangle.h"

#include <libxml/tree.h>
#include <functional>

class TypeTableInfo;

// some members & methods of XcodeMlVisitorBase do not need the info
// of deriving type <Derived>:
// those members & methods are separated from XcodeMlvisitorBase.
// (those methods can be written in the source of XcodeMlVisitorBase.cpp)
class XcodeMlVisitorBaseImpl : public XcodeMlRAVpool {
protected:
    clang::MangleContext *mangleContext;
    xmlNodePtr &rootNode;       // the root node (reference to root visitor).
    xmlNodePtr curNode;         // a candidate of the new chlid.
    TypeTableInfo *typetableinfo;
public:
    XcodeMlVisitorBaseImpl() = delete;
    XcodeMlVisitorBaseImpl(const XcodeMlVisitorBaseImpl&) = delete;
    XcodeMlVisitorBaseImpl(XcodeMlVisitorBaseImpl&&) = delete;
    XcodeMlVisitorBaseImpl& operator =(const XcodeMlVisitorBaseImpl&) = delete;
    XcodeMlVisitorBaseImpl& operator =(XcodeMlVisitorBaseImpl&&) = delete;

    explicit XcodeMlVisitorBaseImpl(clang::MangleContext *MC,
                                    xmlNodePtr &RootNode,
                                    xmlNodePtr CurNode,
                                    TypeTableInfo *TTI);

    void newChild(const char *Name, const char *Contents = nullptr);
    void newProp(const char *Name, int Val, xmlNodePtr N = nullptr);
    void newProp(const char *Name, const char *Val, xmlNodePtr N = nullptr);
    void newComment(const xmlChar *str, xmlNodePtr RN = nullptr);
    void newComment(const char *str, xmlNodePtr RN = nullptr);
    void setLocation(clang::SourceLocation Loc, xmlNodePtr N = nullptr);
};

// Main class: XcodeMlVisitorBase<Derived>
// this is CRTP (Curiously Recurring Template Pattern)
template <class Derived, class OptContext = bool>
class XcodeMlVisitorBase : public XcodeMlVisitorBaseImpl {
protected:
    OptContext optContext;
    llvm::SmallVector<std::function<void (Derived &)>, 5>hooks;
public:
    XcodeMlVisitorBase() = delete;
    XcodeMlVisitorBase(const XcodeMlVisitorBase&) = delete;
    XcodeMlVisitorBase(XcodeMlVisitorBase&&) = delete;
    XcodeMlVisitorBase& operator =(const XcodeMlVisitorBase&) = delete;
    XcodeMlVisitorBase& operator =(XcodeMlVisitorBase&&) = delete;

    explicit XcodeMlVisitorBase(clang::MangleContext *MC, xmlNodePtr &RootNode,
                                const char *ChildName,
                                TypeTableInfo *TTI = nullptr)
        : XcodeMlVisitorBaseImpl(MC, RootNode,
                                 (ChildName
                                  ? xmlNewChild(RootNode, nullptr,
                                                BAD_CAST ChildName, nullptr)
                                  : RootNode),
                                 TTI),
          optContext(), hooks() {};
    explicit XcodeMlVisitorBase(XcodeMlVisitorBase *p)
        : XcodeMlVisitorBaseImpl(p->mangleContext, p->curNode, p->curNode,
                                 p->typetableinfo),
          optContext(p->optContext), hooks() {};

    Derived &getDerived() { return *static_cast<Derived *>(this); }

#define DISPATCHER(NAME, TYPE)                                          \
    bool PreVisit##NAME(TYPE S) {                                       \
        (void)S;                                                        \
        newChild("Traverse" #NAME);                                     \
        return true;                                                    \
    }                                                                   \
    bool Bridge##NAME(TYPE S) override {                                \
        return Traverse##NAME(S);                                       \
    }                                                                   \
    bool Traverse##NAME(TYPE S) {                                       \
        Derived V(this);                                                \
        if (!hooks.empty()) {                                           \
            auto Hook = hooks.back();                                   \
            V.newComment("do Hook" #NAME);                              \
            Hook(V);                                                    \
            hooks.pop_back();                                           \
        }                                                               \
        V.newComment("Traverse" #NAME);                                 \
        if (!V.PreVisit##NAME(S)) {                                     \
            return true; /* avoid traverse children */                  \
        }                                                               \
        return V.otherside->Bridge##NAME(S);                            \
    }

    DISPATCHER(Stmt, clang::Stmt *);
    DISPATCHER(Type, clang::QualType);
    DISPATCHER(TypeLoc, clang::TypeLoc);
    DISPATCHER(Attr, clang::Attr *);
    DISPATCHER(Decl, clang::Decl *);
    DISPATCHER(NestedNameSpecifier, clang::NestedNameSpecifier *);
    DISPATCHER(NestedNameSpecifierLoc, clang::NestedNameSpecifierLoc);
    DISPATCHER(DeclarationNameInfo, clang::DeclarationNameInfo);
    DISPATCHER(TemplateName, clang::TemplateName);
    DISPATCHER(TemplateArgument, const clang::TemplateArgument &);
    DISPATCHER(TemplateArgumentLoc, const clang::TemplateArgumentLoc &);
    DISPATCHER(ConstructorInitializer, clang::CXXCtorInitializer *);
#undef DISPATCHER
};

#endif /* !XCODEMLVISITORBASE_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
