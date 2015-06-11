class TypeTableVisitor
    : public XcodeMlVisitorBase<TypeTableVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    const char *NameForStmt(Stmt *) const;
    const char *NameForDecl(Decl *) const;
};

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
