#ifndef DECLARATIONSVISITOR_H
#define DECLARATIONSVISITOR_H

class DeclarationsVisitor
    : public XMLVisitorBase<DeclarationsVisitor> {
public:
    // use base constructors
    using XMLVisitorBase::XMLVisitorBase;

    const char *getVisitorName() const override;
    bool PreVisitStmt(clang::Stmt *);
    bool PreVisitType(clang::QualType);
    bool PreVisitAttr(clang::Attr *);
    bool PreVisitDecl(clang::Decl *);
    bool PreVisitDeclarationNameInfo(clang::DeclarationNameInfo);
};

#endif /* !DECLARATIONSVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
