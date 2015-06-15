#ifndef DECLARATIONSVISITOR_H
#define DECLARATIONSVISITOR_H

struct DeclarationsContext {
    explicit DeclarationsContext()
	: isInExprStatement(false),
          propname(nullptr),
	  tmpstr() {};
    explicit DeclarationsContext(DeclarationsContext &DC) 
	: isInExprStatement(DC.isInExprStatement),
          propname(nullptr),
	  tmpstr() {};
    DeclarationsContext &operator =(const DeclarationsContext &) = delete;
    DeclarationsContext &operator =(DeclarationsContext &&) = delete;

    bool isInExprStatement;     // inherited to ancestors
    const char *propname;
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

    void WrapChild(const char *name1, const char *name2 = nullptr,
                   const char *name3 = nullptr, const char *name4 = nullptr);
    void PropChild(const char *name);
};

#endif /* !DECLARATIONSVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
