#ifndef XMLRECURSVEXMLVISITOR_H
#define XMLRECURSVEXMLVISITOR_H

#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include "llvm/ADT/SmallVector.h"
#include "clang/AST/Mangle.h"

#include <libxml/tree.h>
#include <functional>
#include <string>

#include "CXXtoXML.h"
#include "NnsTableInfo.h"
#include "TypeTableInfo.h"

#include "InheritanceInfo.h"
#include "XcodeMlNameElem.h"

#include "clang/Basic/Builtins.h"
#include "clang/Lex/Lexer.h"
#include <map>
#include <sstream>

#include "ClangUtil.h"
#include "ClangOperator.h"

using namespace clang;

template <typename Derived>
class ExtendedRecursiveASTVisitor : public RecursiveASTVisitor<Derived> {
protected:
  xmlNodePtr curNode; // a candidate of the new chlid.
public:
  /// \brief Return a reference to the derived class.
  Derived &getDerived() { return *static_cast<Derived *>(this); }

  bool shouldUseDataRecursionFor(Stmt *S) const { return false; }

#define DISPATCHER(NAME, TYPE)					\
public:                                                         \
  bool Traverse##NAME(TYPE S) {                                 \
    xmlNodePtr save = curNode; \
    if(debug_flag) printf("*** push curNode=%p\n",(void *)curNode);      \
    bool ret = RecursiveASTVisitor<Derived>::Traverse##NAME(S); \
    ret &= getDerived().PostVisit##NAME(S);			\
    curNode = save; \
    if(debug_flag) printf("*** pop curNode=%p\n",(void *)curNode);       \
    return ret;                                                 \
  }  \
  bool PostVisit##NAME(TYPE S) {                                \
    (void) S;                                                   \
    return true;                                                \
  }

  DISPATCHER(Stmt, clang::Stmt *);
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
    // DISPATCHER(Type, clang::QualType);
#undef DISPATCHER

    bool TraverseType(clang::QualType S){
        xmlNodePtr save = curNode;
        bool ret = getDerived().VisitType(S);
        ret &= RecursiveASTVisitor<Derived>::TraverseType(S);
        curNode = save;
        return ret;
    }

    bool TraverseDecl(TranslationUnitDecl *D){
        xmlNodePtr save = curNode;
        bool ret = getDerived().VisitDecl(D);
        for (auto *Child : D->decls()) {
            getDerived().TraverseDecl(Child);
        }
        ret &= getDerived().PostVisitDecl(D);
        curNode = save;
        return ret;
    }

  // bool TraverseStmt(Stmt *S) {  
  //   xmlNodePtr save = curNode;
  //   // printf("*** push(Stmt) curNode=%p\n",(void *)curNode);
  //   bool ret = RecursiveASTVisitor<Derived>::TraverseStmt(S); 
  //   curNode = save;
  //   // printf("*** pop(Stmt) curNode=%p\n",(void *)curNode);
  //   return ret;
  // } 

};

//
// Main class: XMLRecursiveASTVisitor
//
class XMLRecursiveASTVisitor : public ExtendedRecursiveASTVisitor<XMLRecursiveASTVisitor> {
 protected:
  clang::MangleContext *mangleContext;
  TypeTableInfo typetableinfo;
  NnsTableInfo nnstableinfo;

 public:
    // constructor
  explicit XMLRecursiveASTVisitor(clang::MangleContext *MC,
				  xmlNodePtr Parent,
				  const char *ChildName,
				  InheritanceInfo *II)
    : mangleContext(MC),
      typetableinfo(MC, II, &nnstableinfo),
      nnstableinfo(MC, &typetableinfo) {
      curNode = ChildName ? xmlNewTextChild(Parent, nullptr, BAD_CAST ChildName, nullptr)
      : Parent;
  }

  // Funtions to maniplate XML
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

  const char *NameForStmt(clang::Stmt *S);
  const char *NameForType(clang::QualType QT);
  const char *NameForTypeLoc(clang::TypeLoc TL);
  const char *NameForAttr(clang::Attr *A);
  const char *NameForDecl(clang::Decl *D);
  const char *NameForNestedNameSpecifier(clang::NestedNameSpecifier *NS);
  const char *NameForNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NL);
  const char *NameForDeclarationName(clang::DeclarationName DN);
  const char *NameForDeclarationNameInfo(clang::DeclarationNameInfo DN);
  const char *NameForTemplateName(clang::TemplateName TN);
  const char *NameForTemplateArgument(const clang::TemplateArgument &TA);
  const char *NameForTemplateArgumentLoc(const clang::TemplateArgumentLoc &TL);
  const char *NameForConstructorInitializer(clang::CXXCtorInitializer *CI);

  bool  SourceLocForStmt(clang::Stmt *S, clang::SourceLocation &SL);
  bool  SourceLocForType(clang::QualType QT, clang::SourceLocation &SL);
  bool  SourceLocForTypeLoc(clang::TypeLoc TL, clang::SourceLocation &SL);
  bool  SourceLocForAttr(clang::Attr *A, clang::SourceLocation &SL);
  bool  SourceLocForDecl(clang::Decl *D, clang::SourceLocation &SL);
  bool  SourceLocForNestedNameSpecifier(
					clang::NestedNameSpecifier *NS, clang::SourceLocation &SL);
  bool  SourceLocForNestedNameSpecifierLoc(
					   clang::NestedNameSpecifierLoc NL, clang::SourceLocation &SL);
  bool  SourceLocForDeclarationNameInfo(
					clang::DeclarationNameInfo DN, clang::SourceLocation &SL);
  bool  SourceLocForTemplateName(clang::TemplateName TN, clang::SourceLocation &SL);
  bool  SourceLocForTemplateArgument(
				     const clang::TemplateArgument &TA, clang::SourceLocation &SL);
  bool  SourceLocForTemplateArgumentLoc(
					const clang::TemplateArgumentLoc &TL, clang::SourceLocation &SL);
  bool
  SourceLocForConstructorInitializer(clang::CXXCtorInitializer *CI, clang::SourceLocation &SL);

#define DEF_ADD_COMMENT(NAME, TYPE)				\
public:                                                         \
  void addComment##NAME(TYPE S) {                               \
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
      if (FullTrace()) {        					      \
        newChild(OS.str().c_str());                                           \
      } else {                                                                \
        newComment(OS.str().c_str());                                         \
      }                                                                       \
    }                                                                         \
  }

  bool
  FullTrace(void) const {
    return false;
  };

#define DEF_VISITOR(NAME, TYPE)					\
public:                                                         \
  bool Visit##NAME(TYPE S) {                                    \
    (void) S;                                                   \
    newChild(#NAME);                                            \
    newProp("class", NameFor##NAME(S));                         \
    clang::SourceLocation SL;                                   \
    if (SourceLocFor##NAME(S, SL)) {                            \
      setLocation(SL);                                          \
    }                                                           \
    return true;   						\
  }

// DISPATCHER(Stmt, clang::Stmt *);
  bool VisitStmt(clang::Stmt *);
//  DISPATCHER(Type, clang::QualType);  QualType -> Type *?
  bool VisitType(clang::QualType);
//   DISPATCHER(TypeLoc, clang::TypeLoc);
  bool VisitTypeLoc(clang::TypeLoc);
//  DISPATCHER(Attr, clang::Attr *);
  bool VisitAttr(clang::Attr *);
//  DISPATCHER(Decl, clang::Decl *);
  bool VisitDecl(clang::Decl *);
  bool PostVisitDecl(clang::Decl *);
//  DISPATCHER(NestedNameSpecifier, clang::NestedNameSpecifier *);
  DEF_VISITOR(NestedNameSpecifier, clang::NestedNameSpecifier *);
//  DISPATCHER(NestedNameSpecifierLoc, clang::NestedNameSpecifierLoc);
  bool VisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc);
//  DISPATCHER(DeclarationNameInfo, clang::DeclarationNameInfo);
  bool VisitDeclarationNameInfo(clang::DeclarationNameInfo);

//  DISPATCHER(TemplateName, clang::TemplateName);
  DEF_VISITOR(TemplateName, clang::TemplateName);
//  DISPATCHER(TemplateArgument, const clang::TemplateArgument &);
  DEF_VISITOR(TemplateArgument, const clang::TemplateArgument &);
//  DISPATCHER(TemplateArgumentLoc, const clang::TemplateArgumentLoc &);
  DEF_VISITOR(TemplateArgumentLoc, const clang::TemplateArgumentLoc &);

//  DISPATCHER(ConstructorInitializer, clang::CXXCtorInitializer *);
  bool VisitConstructorInitializer(clang::CXXCtorInitializer *);
#undef DEF_ADD_COMMENT
#undef DEF_VISITOR

    bool VisitType(Type* S) {
        (void) S;
        // S->dump();
        return true;  
    }

};

#endif /* !XMLRECURSIVEASTVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
