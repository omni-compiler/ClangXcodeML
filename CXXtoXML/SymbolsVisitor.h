#ifndef SYMBOLVISITOR_H
#define SYMBOLVISITOR_H

class SymbolsVisitor
    : public XMLVisitorBase<SymbolsVisitor> {
public:
    // use base constructors
    using XMLVisitorBase::XMLVisitorBase;

    const char *getVisitorName() const override;
    bool PreVisitStmt(clang::Stmt *);
    bool PreVisitType(clang::QualType);
    bool PreVisitTypeLoc(clang::TypeLoc);
    bool PreVisitAttr(clang::Attr *);
    bool PreVisitDecl(clang::Decl *);
    bool PreVisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc);
    bool PreVisitDeclarationNameInfo(clang::DeclarationNameInfo);
    bool PreVisitConstructorInitializer(clang::CXXCtorInitializer *CI);
};

#endif /* !SYMBOLVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
