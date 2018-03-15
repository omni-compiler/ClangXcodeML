#ifndef DECLARATIONSVISITOR_H
#define DECLARATIONSVISITOR_H

class DeclarationsVisitor : public XMLVisitorBase<DeclarationsVisitor> {
public:
  // use base constructors
  using XMLVisitorBase::XMLVisitorBase;

  const char *getVisitorName() const override;
  bool PreVisitStmt(clang::Stmt *);
  bool PreVisitType(clang::QualType);
  bool PreVisitTypeLoc(clang::TypeLoc);
  bool PreVisitAttr(clang::Attr *);
  bool PreVisitDecl(clang::Decl *);
  bool PostVisitDecl(clang::Decl *);
  bool PreVisitDeclarationNameInfo(clang::DeclarationNameInfo);
  bool PreVisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc);
  bool PreVisitConstructorInitializer(clang::CXXCtorInitializer *);
};

class InheritanceInfo;

class DeclarationsVisitorContext {
public:
  DeclarationsVisitorContext(clang::MangleContext *MC, InheritanceInfo *II)
      : typetableinfo(MC, II), nnstableinfo(MC, &typetableinfo) {
  }
  TypeTableInfo typetableinfo;
  NnsTableInfo nnstableinfo;
};

#endif /* !DECLARATIONSVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
