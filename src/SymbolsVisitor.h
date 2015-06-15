#ifndef SYMBOLVISITOR_H
#define SYMBOLVISITOR_H

class SymbolsVisitor
    : public XcodeMlVisitorBase<SymbolsVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    bool PreVisitStmt(clang::Stmt *);
    bool PreVisitDecl(clang::Decl *);
};

#endif /* !SYMBOLVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
