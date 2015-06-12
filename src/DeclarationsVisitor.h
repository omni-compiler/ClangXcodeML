struct DeclarationsContext {
    bool isInCompoundStatement = false;
    bool isInExprStatement = false;
};

class DeclarationsVisitor
    : public XcodeMlVisitorBase<DeclarationsVisitor, DeclarationsContext> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    const char *NameForStmt(Stmt *);
    const char *NameForType(QualType);
    const char *NameForTypeLoc(TypeLoc);
    const char *NameForAttr(Attr *);
    const char *NameForDecl(Decl *);
    const char *NameForNestedNameSpecifier(NestedNameSpecifier *);
    const char *NameForNestedNameSpecifierLoc(NestedNameSpecifierLoc);
    const char *NameForDeclarationNameInfo(DeclarationNameInfo);
    const char *ContentsForDeclarationNameInfo(DeclarationNameInfo);
};

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
