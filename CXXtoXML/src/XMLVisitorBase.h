#ifndef XMLVISITORBASE_H
#define XMLVISITORBASE_H

#include "XMLRAV.h"
#include "llvm/ADT/SmallVector.h"
#include "clang/AST/Mangle.h"

#include <libxml/tree.h>
#include <functional>
#include <string>

class TypeTableInfo;
class NnsTableInfo;

// some members & methods of XMLVisitorBase do not need the info
// of deriving type <Derived>:
// those members & methods are separated from XMLvisitorBase.
// (those methods can be written in the source of XMLVisitorBase.cpp)
class XMLVisitorBaseImpl : public XMLRAVpool {
protected:
  clang::MangleContext *mangleContext;
  xmlNodePtr curNode; // a candidate of the new chlid.

public:
  XMLVisitorBaseImpl() = delete;
  XMLVisitorBaseImpl(const XMLVisitorBaseImpl &) = delete;
  XMLVisitorBaseImpl(XMLVisitorBaseImpl &&) = delete;
  XMLVisitorBaseImpl &operator=(const XMLVisitorBaseImpl &) = delete;
  XMLVisitorBaseImpl &operator=(XMLVisitorBaseImpl &&) = delete;

  explicit XMLVisitorBaseImpl(clang::MangleContext *MC, xmlNodePtr CurNode);

  xmlNodePtr addChild(const char *Name, const char *Content = nullptr);
  void newChild(const char *Name, const char *Content = nullptr);
  void newProp(const char *Name, int Val, xmlNodePtr N = nullptr);
  void newProp(const char *Name, const char *Val, xmlNodePtr N = nullptr);
  void newBoolProp(const char *Name, bool Val, xmlNodePtr N = nullptr);
  void newComment(const xmlChar *str, xmlNodePtr RN = nullptr);
  void newComment(const char *str, xmlNodePtr RN = nullptr);
  void newComment(const std::string &str, xmlNodePtr RN = nullptr);
  void setLocation(clang::SourceLocation Loc, xmlNodePtr N = nullptr);
  std::string contentBySource(
      clang::SourceLocation LocStart, clang::SourceLocation LocEnd);
};

// Main class: XMLVisitorBase<Derived>
// this is CRTP (Curiously Recurring Template Pattern)
template <class Derived, class OptContext = bool>
class XMLVisitorBase : public XMLVisitorBaseImpl {
protected:
  OptContext optContext;

public:
  XMLVisitorBase() = delete;
  XMLVisitorBase(const XMLVisitorBase &) = delete;
  XMLVisitorBase(XMLVisitorBase &&) = delete;
  XMLVisitorBase &operator=(const XMLVisitorBase &) = delete;
  XMLVisitorBase &operator=(XMLVisitorBase &&) = delete;

  explicit XMLVisitorBase(clang::MangleContext *MC,
      xmlNodePtr Parent,
      const char *ChildName,
      const OptContext &OC)
      : XMLVisitorBaseImpl(MC,
            (ChildName ? xmlNewTextChild(
                             Parent, nullptr, BAD_CAST ChildName, nullptr)
                       : Parent)),
        optContext(OC){};
  explicit XMLVisitorBase(XMLVisitorBase *p)
      : XMLVisitorBaseImpl(p->mangleContext, p->curNode),
        optContext(p->optContext){};

  Derived &
  getDerived() {
    return *static_cast<Derived *>(this);
  }

#define DISPATCHER(NAME, TYPE)                                                \
public:                                                                       \
  bool Bridge##NAME(TYPE S) override {                                        \
    return getDerived().Traverse##NAME(S);                                    \
  }                                                                           \
  bool Traverse##NAME(TYPE S) {                                               \
    Derived V(this);                                                          \
    return V.TraverseMe##NAME(S);                                             \
  }                                                                           \
  bool TraverseMe##NAME(TYPE S) {                                             \
    std::string comment("Traverse" #NAME ":");                                \
    llvm::raw_string_ostream OS(comment);                                     \
    OS << NameFor##NAME(S);                                                   \
    clang::SourceLocation SL;                                                 \
    if (SourceLocFor##NAME(S, SL)) {                                          \
      clang::FullSourceLoc FL;                                                \
      FL = mangleContext->getASTContext().getFullLoc(SL);                     \
      if (FL.isValid()) {                                                     \
        clang::PresumedLoc PL;                                                \
        PL = FL.getManager().getPresumedLoc(FL);                              \
        OS << ":" << PL.getLine() << ":" << PL.getColumn();                   \
      }                                                                       \
      if (getDerived().FullTrace()) {                                         \
        newChild(OS.str().c_str());                                           \
      } else {                                                                \
        newComment(OS.str().c_str());                                         \
      }                                                                       \
    }                                                                         \
    if (!getDerived().PreVisit##NAME(S)) {                                    \
      return true; /* avoid traverse children */                              \
    }                                                                         \
    bool ret = getDerived().TraverseChildOf##NAME(S);                         \
    ret &= getDerived().PostVisit##NAME(S);                                   \
    return ret;                                                               \
  }                                                                           \
  bool TraverseChildOf##NAME(TYPE S) {                                        \
    getDerived().otherside->Bridge##NAME(S);                                  \
    return true;                                                              \
  }                                                                           \
  bool PreVisit##NAME(TYPE S) {                                               \
    (void) S;                                                                 \
    newChild(#NAME);                                                          \
    newProp("class", NameFor##NAME(S));                                       \
    clang::SourceLocation SL;                                                 \
    if (SourceLocFor##NAME(S, SL)) {                                          \
      setLocation(SL);                                                        \
    }                                                                         \
    return getDerived().Visit##NAME(S);                                       \
  }                                                                           \
  bool Visit##NAME(TYPE S) {                                                  \
    (void) S;                                                                 \
    return true;                                                              \
  }                                                                           \
  bool PostVisit##NAME(TYPE S) {                                              \
    (void) S;                                                                 \
    return true;                                                              \
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

  bool
  FullTrace(void) const {
    return false;
  };

  const char *
  NameForStmt(clang::Stmt *S) {
    return S ? S->getStmtClassName() : "NULL";
  }
  const char *
  NameForType(clang::QualType QT) {
    return !QT.isNull() ? QT->getTypeClassName() : "NULL";
  }
  const char *
  NameForTypeLoc(clang::TypeLoc TL) {
    return !TL.isNull() ? TL.getType()->getTypeClassName() : "NULL";
  }
  const char *
  NameForAttr(clang::Attr *A) {
    return A ? A->getSpelling() : "NULL";
  }
  const char *
  NameForDecl(clang::Decl *D) {
    return D ? D->getDeclKindName() : "NULL";
  }
  const char *
  NameForNestedNameSpecifier(clang::NestedNameSpecifier *NS) {
    if (!NS)
      return "NULL";
    switch (NS->getKind()) {
    case clang::NestedNameSpecifier::Identifier: return "Identifier";
    case clang::NestedNameSpecifier::Namespace: return "Namespace";
    case clang::NestedNameSpecifier::NamespaceAlias: return "NamespaceAlias";
    case clang::NestedNameSpecifier::Global: return "Global";
    case clang::NestedNameSpecifier::Super: return "Super";
    case clang::NestedNameSpecifier::TypeSpec: return "TypeSpec";
    case clang::NestedNameSpecifier::TypeSpecWithTemplate:
      return "TypeSpecWithTemplate";
    }
  }
  const char *
  NameForNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NL) {
    return NameForNestedNameSpecifier(NL.getNestedNameSpecifier());
  }
  const char *
  NameForDeclarationName(clang::DeclarationName DN) {
    switch (DN.getNameKind()) {
    case clang::DeclarationName::CXXConstructorName:
      return "CXXConstructorName";
    case clang::DeclarationName::CXXDestructorName: return "CXXDestructorName";
    case clang::DeclarationName::CXXConversionFunctionName:
      return "CXXConversionFunctionName";
    case clang::DeclarationName::Identifier: return "Identifier";
    case clang::DeclarationName::ObjCZeroArgSelector:
      return "ObjCZeroArgSelector";
    case clang::DeclarationName::ObjCOneArgSelector:
      return "ObjCOneArgSelector";
    case clang::DeclarationName::ObjCMultiArgSelector:
      return "ObjCMultiArgSelector";
    case clang::DeclarationName::CXXOperatorName: return "CXXOperatorName";
    case clang::DeclarationName::CXXLiteralOperatorName:
      return "CXXLiteralOperatorName";
    case clang::DeclarationName::CXXUsingDirective: return "CXXUsingDirective";
    default:
        return "Unknown";
    }
  }
  const char *
  NameForDeclarationNameInfo(clang::DeclarationNameInfo DN) {
    return NameForDeclarationName(DN.getName());
  }
  const char *
  NameForTemplateName(clang::TemplateName TN) {
    (void)TN;
    return "X";
  }
  const char *
  NameForTemplateArgument(const clang::TemplateArgument &TA) {
    (void)TA;
    return "X";
  }
  const char *
  NameForTemplateArgumentLoc(const clang::TemplateArgumentLoc &TL) {
    (void)TL;
    return "X";
  }
  const char *
  NameForConstructorInitializer(clang::CXXCtorInitializer *CI) {
    (void)CI;
    return "X";
  }

  bool
  SourceLocForStmt(clang::Stmt *S, clang::SourceLocation &SL) {
    if (S) {
      SL = S->getBeginLoc();
      return true;
    } else {
      return false;
    }
  }
  bool
  SourceLocForType(clang::QualType QT, clang::SourceLocation &SL) {
    (void)QT, (void)SL;
    return false;
  }
  bool
  SourceLocForTypeLoc(clang::TypeLoc TL, clang::SourceLocation &SL) {
    SL = TL.getBeginLoc();
    return true;
  }
  bool
  SourceLocForAttr(clang::Attr *A, clang::SourceLocation &SL) {
    if (A) {
      SL = A->getLocation();
      return true;
    } else {
      return false;
    }
  }
  bool
  SourceLocForDecl(clang::Decl *D, clang::SourceLocation &SL) {
    if (D) {
      SL = D->getBeginLoc();
      return true;
    } else {
      return false;
    }
  }
  bool
  SourceLocForNestedNameSpecifier(
      clang::NestedNameSpecifier *NS, clang::SourceLocation &SL) {
    (void)NS;
    (void)SL;
    return false;
  }
  bool
  SourceLocForNestedNameSpecifierLoc(
      clang::NestedNameSpecifierLoc NL, clang::SourceLocation &SL) {
    SL = NL.getLocalBeginLoc();
    return true;
  }
  bool
  SourceLocForDeclarationNameInfo(
      clang::DeclarationNameInfo DN, clang::SourceLocation &SL) {
    SL = DN.getBeginLoc();
    return true;
  }
  bool
  SourceLocForTemplateName(clang::TemplateName TN, clang::SourceLocation &SL) {
    (void)TN;
    (void)SL;
    return false;
  }
  bool
  SourceLocForTemplateArgument(
      const clang::TemplateArgument &TA, clang::SourceLocation &SL) {
    (void)TA;
    (void)SL;
    return false;
  }
  bool
  SourceLocForTemplateArgumentLoc(
      const clang::TemplateArgumentLoc &TL, clang::SourceLocation &SL) {
    (void)TL;
    (void)SL;
    return false;
  }
  bool
  SourceLocForConstructorInitializer(
      clang::CXXCtorInitializer *CI, clang::SourceLocation &SL) {
    (void)CI;
    (void)SL;
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
