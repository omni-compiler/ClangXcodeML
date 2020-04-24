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
  using BASE = RecursiveASTVisitor<Derived>;
protected:
  xmlNodePtr curNode; // a candidate of the new chlid.
public:
  /// \brief Return a reference to the derived class.
  Derived &getDerived() { return *static_cast<Derived *>(this); }

  bool shouldUseDataRecursionFor(Stmt *S) const { return false; }
  bool shouldVisitTemplateInstantiations() const {return false;}
#define DISPATCHER(NAME, TYPE)					\
public:                                                         \
  bool Traverse##NAME(TYPE S) {                                 \
    xmlNodePtr save = curNode; \
    if(CXXtoXML::debug_flag) printf("*** push curNode=%p\n",(void *)curNode); \
    bool ret = RecursiveASTVisitor<Derived>::Traverse##NAME(S); \
    ret &= getDerived().PostVisit##NAME(S);			\
    curNode = save; \
    if(CXXtoXML::debug_flag) printf("*** pop curNode=%p\n",(void *)curNode); \
    return ret;                                                 \
  }  \
  bool PostVisit##NAME(TYPE S) {                                \
    (void) S;                                                   \
    return true;                                                \
  }

  DISPATCHER(Stmt, clang::Stmt *);
  DISPATCHER(TypeLoc, clang::TypeLoc);
  DISPATCHER(Attr, clang::Attr *);
    // DISPATCHER(Decl, clang::Decl *);
  bool PreVisitDecl(clang::Decl *S) {
    (void) S;
    return true;
  }
  bool TraverseDecl(clang::Decl *S) {
        xmlNodePtr save = curNode;
    if(CXXtoXML::debug_flag) printf("*** push curNode=%p\n",(void *)curNode);
    getDerived().PreVisitDecl(S);
    bool ret = RecursiveASTVisitor<Derived>::TraverseDecl(S);
    ret &= getDerived().PostVisitDecl(S);
    curNode = save;
    if(CXXtoXML::debug_flag) printf("*** pop curNode=%p\n",(void *)curNode);
    return ret;
  }
  bool PostVisitDecl(clang::Decl *S) {
    (void) S;
    return true;
  }
  DISPATCHER(NestedNameSpecifier, clang::NestedNameSpecifier *);
    //  DISPATCHER(NestedNameSpecifierLoc, clang::NestedNameSpecifierLoc);
    //DISPATCHER(DeclarationNameInfo, clang::DeclarationNameInfo);
  DISPATCHER(TemplateName, clang::TemplateName);
    //DISPATCHER(TemplateArgument, const clang::TemplateArgument &);
    //DISPATCHER(TemplateArgumentLoc, const clang::TemplateArgumentLoc &);
    // DISPATCHER(Type, clang::QualType);
#undef DISPATCHER
    bool VisitConstructorInitializer(clang::CXXCtorInitializer *CI) {
        return true;
    }

  bool TraverseConstructorInitializer(clang::CXXCtorInitializer *CI){
      auto save = curNode;
      getDerived().VisitConstructorInitializer(CI);
      RecursiveASTVisitor<Derived>::TraverseConstructorInitializer(CI);
      curNode = save;
      return true;
  }
    bool VisitTemplateArgumentLoc(clang::TemplateArgumentLoc &AL)
    {
        return true;
    }
    bool VisitTemplateArgument(clang::TemplateArgument &Arg)
    {
        return true;
    }
    bool VisitDeclarationNameInfo(clang::DeclarationNameInfo DNI)
    {
        return true;
    }
    bool TraverseTemplateArgument(const clang::TemplateArgument &TA){
        auto save = curNode;
        getDerived().VisitTemplateArgument(TA);
        BASE::TraverseTemplateArgument(TA);
        curNode = save;
        return true;
    }
    bool TraverseTemplateArgumentLoc(const clang::TemplateArgumentLoc &ArgLoc){
        auto save = curNode;
        getDerived().VisitTemplateArgumentLoc(ArgLoc);
        BASE::TraverseTemplateArgumentLoc(ArgLoc);
        curNode = save;
        return true;
    }
    bool TraverseDeclarationNameInfo(clang::DeclarationNameInfo DNI){
        auto save = curNode;
        getDerived().VisitDeclarationNameInfo(DNI);
        BASE::TraverseDeclarationNameInfo(DNI);
        curNode = save;
        return true;
    }
    bool TraverseType(clang::QualType S){
        xmlNodePtr save = curNode;
        bool ret = getDerived().VisitType(S);
        ret &= RecursiveASTVisitor<Derived>::TraverseType(S);
        curNode = save;
        return true;
    }
#if 0
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
#endif
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
  bool TraverseForStmt(ForStmt *S)
  {
        xmlNodePtr  save = curNode;
        WalkUpFromForStmt(S);
        const std::vector<std::tuple<const char *, Stmt *>>
            children = {
                        std::make_tuple("init", S->getInit()),
                        std::make_tuple("cond", S->getCond()),
                        std::make_tuple("iter", S->getInc()),
                        std::make_tuple("body", S->getBody()),
        };
        for (auto &child : children) {
            const char *kind;
            Stmt *stmt;
            std::tie(kind, stmt) = child;
            if (stmt) {
                TraverseStmt(stmt);
                xmlNewProp(xmlGetLastChild(curNode),
                           BAD_CAST "for_stmt_kind", BAD_CAST kind);
            }
        }
        curNode = save;
        if(CXXtoXML::debug_flag) printf("*** pop curNode=%p\n",(void *)curNode);
        return true;
    }
  bool TraverseInitListExpr(InitListExpr *ILE)
  {
    WalkUpFromInitListExpr(ILE);
    for (auto& range : ILE->children()) {
          TraverseStmt(range);
    }
    return true;
  }
  bool TraverseCXXDefaultArgExpr(CXXDefaultArgExpr *CDAE)
  {
    WalkUpFromCXXDefaultArgExpr(CDAE);
    const auto E = CDAE->getExpr();
    TraverseStmt(E);
    return true;
  }
  bool TraverseUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *UEOTTE)
  {
    WalkUpFromUnaryExprOrTypeTraitExpr(UEOTTE);
    switch (UEOTTE->getKind()) {
    case UETT_SizeOf: {
      newChild("sizeOfExpr");
      TraverseType(static_cast<Expr *>(UEOTTE)->getType());
      if (UEOTTE->isArgumentType()) {
        newChild("typeName");
        TraverseType(UEOTTE->getArgumentType());
        break;
      } else {
        TraverseStmt(UEOTTE->getArgumentExpr());
        break; // already traversed
      }
    }
    case UETT_PreferredAlignOf:
    case UETT_AlignOf: {
        newChild((UEOTTE->getKind()==UETT_PreferredAlignOf) ?
                 "gccAlignOfExpr" : "AlignOfExpr");
      TraverseType(static_cast<Expr *>(UEOTTE)->getType());
      if (UEOTTE->isArgumentType()) {
        newChild("typeName");
        TraverseType(UEOTTE->getArgumentType());
      } else {
        TraverseStmt(UEOTTE->getArgumentExpr());
      }
      break;
    }
    case UETT_VecStep:
      newChild("clangStmt");
      newProp("class", "UnaryExprOrTypeTraitExpr_UETT_VecStep");
      break;

    case UETT_OpenMPRequiredSimdAlign:
      //  NStmt("UnaryExprOrTypeTraitExpr(UETT_OpenMPRequiredSimdAlign");
    default:
        UEOTTE->dump();
        abort();
    }
    if(UEOTTE->isArgumentType())
        TraverseTypeLoc(UEOTTE->getArgumentTypeInfo()->getTypeLoc());

    return true ;
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
  bool PreVisitDecl(clang::Decl *);
  bool PostVisitDecl(clang::Decl *);
//  DISPATCHER(NestedNameSpecifier, clang::NestedNameSpecifier *);
  DEF_VISITOR(NestedNameSpecifier, clang::NestedNameSpecifier *);
  bool TraverseNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc);
//  DISPATCHER(DeclarationNameInfo, clang::DeclarationNameInfo);
  bool VisitDeclarationNameInfo(clang::DeclarationNameInfo);

//  DISPATCHER(TemplateName, clang::TemplateName);
  DEF_VISITOR(TemplateName, clang::TemplateName);
//  DISPATCHER(TemplateArgument, const clang::TemplateArgument &);
  DEF_VISITOR(TemplateArgument, const clang::TemplateArgument &);
//  DISPATCHER(TemplateArgumentLoc, const clang::TemplateArgumentLoc &);
  DEF_VISITOR(TemplateArgumentLoc, const clang::TemplateArgumentLoc &);

#undef DEF_ADD_COMMENT
#undef DEF_VISITOR
    bool VisitConstructorInitializer(clang::CXXCtorInitializer *);

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
