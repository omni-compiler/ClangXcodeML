struct DeclarationsContext {
    explicit DeclarationsContext()
	: isInExprStatement(false),
	  children(),
	  sibling(children),
	  tmpstr() {};
    explicit DeclarationsContext(DeclarationsContext &DC) 
	: isInExprStatement(DC.isInExprStatement),
	  children(),
	  sibling(DC.children),
	  tmpstr() {};
    DeclarationsContext &operator =(const DeclarationsContext &) = delete;
    DeclarationsContext &operator =(DeclarationsContext &&) = delete;

    bool isInExprStatement;     // inherited to ancestors
    llvm::SmallVector<const char *, 8> children;
    llvm::SmallVector<const char *, 8> &sibling;
    std::string tmpstr;
};

class DeclarationsVisitor
    : public XcodeMlVisitorBase<DeclarationsVisitor, DeclarationsContext> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    bool PreVisitStmt(clang::Stmt *);
    bool PreVisitType(clang::QualType);
    bool PreVisitTypeLoc(clang::TypeLoc);
    bool PreVisitAttr(clang::Attr *);
    bool PreVisitDecl(clang::Decl *);
    bool PreVisitNestedNameSpecifier(clang::NestedNameSpecifier *);
    bool PreVisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc);
    bool PreVisitDeclarationNameInfo(clang::DeclarationNameInfo);
};

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
