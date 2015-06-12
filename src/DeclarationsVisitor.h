struct DeclarationsContext {
    explicit DeclarationsContext()
	: isInCompoundStatement(false),
	  isInExprStatement(false),
	  children(),
	  sibling(children),
	  tmpstr() {};
    explicit DeclarationsContext(DeclarationsContext &DC) 
	: isInCompoundStatement(DC.isInCompoundStatement),
	  isInExprStatement(DC.isInExprStatement),
	  children(),
	  sibling(DC.children),
	  tmpstr() {};
    DeclarationsContext &operator =(const DeclarationsContext &) = delete;
    DeclarationsContext &operator =(DeclarationsContext &&) = delete;

    bool isInCompoundStatement; // inherited to ancestors
    bool isInExprStatement;     // inherited to ancestors
    SmallVector<const char *, 8> children;
    SmallVector<const char *, 8> &sibling;
    std::string tmpstr;
};

class DeclarationsVisitor
    : public XcodeMlVisitorBase<DeclarationsVisitor, DeclarationsContext> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    const char *NameForStmt(Stmt *);
    bool PreVisitStmt(Stmt *);
    const char *ContentsForStmt(Stmt *);
    const char *NameForType(QualType);
    const char *NameForTypeLoc(TypeLoc);
    const char *NameForAttr(Attr *);
    const char *NameForDecl(Decl *);
    bool PreVisitDecl(Decl *);
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
