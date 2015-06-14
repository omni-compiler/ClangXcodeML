class SymbolsVisitor
    : public XcodeMlVisitorBase<SymbolsVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    const char *NameForDecl(Decl *);
    const char *NameForStmt(Stmt *);
};

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
