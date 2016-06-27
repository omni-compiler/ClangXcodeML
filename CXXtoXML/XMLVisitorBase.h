#ifndef XMLVISITORBASE_H
#define XMLVISITORBASE_H

#include "XMLRAV.h"
#include "llvm/ADT/SmallVector.h"
#include "clang/AST/Mangle.h"

#include <libxml/tree.h>
#include <functional>
#include <string>

class TypeTableInfo;

// some members & methods of XMLVisitorBase do not need the info
// of deriving type <Derived>:
// those members & methods are separated from XMLvisitorBase.
// (those methods can be written in the source of XMLVisitorBase.cpp)
class XMLVisitorBaseImpl : public XMLRAVpool {
protected:
    clang::MangleContext *mangleContext;
    xmlNodePtr curNode;        // a candidate of the new chlid.
    TypeTableInfo *typetableinfo;
public:
    XMLVisitorBaseImpl() = delete;
    XMLVisitorBaseImpl(const XMLVisitorBaseImpl&) = delete;
    XMLVisitorBaseImpl(XMLVisitorBaseImpl&&) = delete;
    XMLVisitorBaseImpl& operator =(const XMLVisitorBaseImpl&) = delete;
    XMLVisitorBaseImpl& operator =(XMLVisitorBaseImpl&&) = delete;

    explicit XMLVisitorBaseImpl(clang::MangleContext *MC,
                                xmlNodePtr CurNode,
                                TypeTableInfo *TTI);

    xmlNodePtr addChild(const char *Name, const char *Content = nullptr);
    xmlNodePtr addChild(const char *Name, xmlNodePtr N);
    xmlNodePtr addName(clang::NamedDecl *ND, const char *Content = nullptr);
    xmlNodePtr addName(const char *FullName, const char *Content = nullptr);
    void newChild(const char *Name, const char *Content = nullptr);
    void newName(clang::NamedDecl *ND, const char *Content = nullptr);
    void newName(const char *FullName, const char *Content = nullptr);
    void newProp(const char *Name, int Val, xmlNodePtr N = nullptr);
    void newProp(const char *Name, const char *Val, xmlNodePtr N = nullptr);
    void newComment(const xmlChar *str, xmlNodePtr RN = nullptr);
    void newComment(const char *str, xmlNodePtr RN = nullptr);
    void newComment(const std::string &str, xmlNodePtr RN = nullptr);
    void setLocation(clang::SourceLocation Loc, xmlNodePtr N = nullptr);
    std::string contentBySource(clang::SourceLocation LocStart,
                                clang::SourceLocation LocEnd);
};

// Main class: XMLVisitorBase<Derived>
// this is CRTP (Curiously Recurring Template Pattern)
template <class Derived, class OptContext = bool>
class XMLVisitorBase : public XMLVisitorBaseImpl {
protected:
    OptContext optContext;
public:
    XMLVisitorBase() = delete;
    XMLVisitorBase(const XMLVisitorBase&) = delete;
    XMLVisitorBase(XMLVisitorBase&&) = delete;
    XMLVisitorBase& operator =(const XMLVisitorBase&) = delete;
    XMLVisitorBase& operator =(XMLVisitorBase&&) = delete;

    explicit XMLVisitorBase(clang::MangleContext *MC, xmlNodePtr Parent,
                            const char *ChildName,
                            TypeTableInfo *TTI = nullptr)
        : XMLVisitorBaseImpl(MC, (ChildName
                                  ? xmlNewTextChild(Parent, nullptr,
                                                    BAD_CAST ChildName,
                                                    nullptr)
                                  : Parent),
                             TTI),
          optContext() {};
    explicit XMLVisitorBase(XMLVisitorBase *p)
        : XMLVisitorBaseImpl(p->mangleContext, p->curNode,
                             p->typetableinfo),
          optContext(p->optContext) {};

    Derived &getDerived() { return *static_cast<Derived *>(this); }

#define DISPATCHER(NAME, TYPE)                                          \
    public:                                                             \
    std::function<bool (TYPE)>HookFor##NAME;                            \
    bool PreVisit##NAME(TYPE S) {                                       \
        (void)S;                                                        \
        newChild("Traverse" #NAME);                                     \
        return true;                                                    \
    }                                                                   \
    bool Bridge##NAME(TYPE S) override {                                \
        if (HookFor##NAME) {                                            \
            newComment("do Hook " #NAME);                               \
            return HookFor##NAME(S);                                    \
        } else {                                                        \
            return getDerived().Traverse##NAME(S);                      \
        }                                                               \
    }                                                                   \
    bool Traverse##NAME(TYPE S) {                                       \
        Derived V(this);                                                \
        return V.TraverseMe##NAME(S);                                   \
    }                                                                   \
    bool TraverseMe##NAME(TYPE S) {                                     \
        std::string comment("Traverse" #NAME ":");                      \
        llvm::raw_string_ostream OS(comment);                           \
        OS << NameFor##NAME(S);                                         \
        clang::SourceLocation SL;                                       \
        if (SourceLocFor##NAME(S, SL)) {                                \
            clang::FullSourceLoc FL;                                    \
            FL = mangleContext->getASTContext().getFullLoc(SL);         \
            if (FL.isValid()) {                                         \
                clang::PresumedLoc PL;                                  \
                PL = FL.getManager().getPresumedLoc(FL);                \
                OS << ":" << PL.getLine() << ":" << PL.getColumn();     \
            }                                                           \
            if (getDerived().FullTrace()) {                             \
                newChild(OS.str().c_str());                             \
            } else {                                                    \
                newComment(OS.str().c_str());                           \
            }                                                           \
        }                                                               \
        if (!getDerived().PreVisit##NAME(S)) {                          \
            return true; /* avoid traverse children */                  \
        }                                                               \
        return getDerived().TraverseChildOf##NAME(S);                   \
    }                                                                   \
    bool TraverseChildOf##NAME(TYPE S) {                                \
        getDerived().otherside->Bridge##NAME(S);                        \
        return true;                                                    \
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

    bool FullTrace(void) const { return false; };

    const char *NameForStmt(clang::Stmt *S) {
        return S ? S->getStmtClassName() : "";
    }
    const char *NameForType(clang::QualType QT) {
        return QT->getTypeClassName();
    }
    const char *NameForTypeLoc(clang::TypeLoc TL) {
        return TL.getType()->getTypeClassName();
    }
    const char *NameForAttr(clang::Attr *A) {
        return A ? A->getSpelling() : "";
    }
    const char *NameForDecl(clang::Decl *D) {
        return D ? D->getDeclKindName(): ""; 
    }
    const char *NameForNestedNameSpecifier(clang::NestedNameSpecifier *NS) {
        (void)NS; return "X";
    }
    const char *NameForNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NL) {
        (void)NL; return "X";
    }
    const char *NameForDeclarationNameInfo(clang::DeclarationNameInfo DN) {
        (void)DN; return "X";
    }
    const char *NameForTemplateName(clang::TemplateName TN) {
        (void)TN; return "X";
    }
    const char *NameForTemplateArgument(const clang::TemplateArgument &TA) {
        (void)TA; return "X";
    }
    const char *NameForTemplateArgumentLoc(const clang::TemplateArgumentLoc &TL) {
        (void)TL; return "X";
    }
    const char *NameForConstructorInitializer(clang::CXXCtorInitializer *CI) {
        (void)CI; return "X";
    }

    bool SourceLocForStmt(clang::Stmt *S, clang::SourceLocation &SL) {
        if (S) {
            SL = S->getLocStart();
            return true;
        } else {
            return false;
        }
    }
    bool SourceLocForType(clang::QualType QT, clang::SourceLocation &SL) {
        (void)QT, (void)SL;
        return false;
    }
    bool SourceLocForTypeLoc(clang::TypeLoc TL, clang::SourceLocation &SL) {
        SL = TL.getLocStart();
        return true;
    }
    bool SourceLocForAttr(clang::Attr *A, clang::SourceLocation &SL) {
        if (A) {
            SL = A->getLocation();
            return true;
        } else {
            return false;
        }
    }
    bool SourceLocForDecl(clang::Decl *D, clang::SourceLocation &SL) {
        if (D) {
            SL = D->getLocStart();
            return true;
        } else {
            return false;
        }
    }
    bool SourceLocForNestedNameSpecifier(clang::NestedNameSpecifier *NS,
                                         clang::SourceLocation &SL) {
        (void)NS; (void)SL;
        return false;
    }
    bool SourceLocForNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NL,
                                            clang::SourceLocation &SL) {
        SL = NL.getLocalBeginLoc();
        return true;
    }
    bool SourceLocForDeclarationNameInfo(clang::DeclarationNameInfo DN,
                                         clang::SourceLocation &SL) {
        SL = DN.getLocStart();
        return true;
    }
    bool SourceLocForTemplateName(clang::TemplateName TN,
                                  clang::SourceLocation &SL) {
        (void)TN; (void)SL;
        return false;
    }
    bool SourceLocForTemplateArgument(const clang::TemplateArgument &TA,
                                      clang::SourceLocation &SL) {
        (void)TA; (void)SL;
        return false;
    }
    bool SourceLocForTemplateArgumentLoc(const clang::TemplateArgumentLoc &TL,
                                         clang::SourceLocation &SL) {
        (void)TL; (void)SL;
        return false;
    }
    bool SourceLocForConstructorInitializer(clang::CXXCtorInitializer *CI,
                                            clang::SourceLocation &SL) {
        (void)CI; (void)SL;
        return false;
    }
};

#endif /* !XMLVISITORBASE_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
