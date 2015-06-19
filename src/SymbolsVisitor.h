#ifndef SYMBOLVISITOR_H
#define SYMBOLVISITOR_H

class SymbolsVisitor
    : public XcodeMlVisitorBase<SymbolsVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    bool PreVisitStmt(clang::Stmt *);
    bool PreVisitTypeLoc(clang::TypeLoc);
    bool PreVisitAttr(clang::Attr *);
    bool PreVisitDecl(clang::Decl *);
    bool PreVisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc);
    bool PreVisitDeclarationNameInfo(clang::DeclarationNameInfo);
};

#endif /* !SYMBOLVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
