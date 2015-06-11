class DeclarationsVisitor
    : public XcodeMlVisitorBase<DeclarationsVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    const char *NameForStmt(Stmt *) const;
    const char *NameForType(QualType) const;
    const char *NameForTypeLoc(TypeLoc) const;
    const char *NameForAttr(Attr *) const;
    const char *NameForDecl(Decl *) const;
    const char *NameForNestedNameSpecifier(NestedNameSpecifier *) const;
    const char *NameForNestedNameSpecifierLoc(NestedNameSpecifierLoc) const;
    const char *NameForDeclarationNameInfo(DeclarationNameInfo) const;
    const char *ContentsForDeclarationNameInfo(DeclarationNameInfo) const;
};

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
