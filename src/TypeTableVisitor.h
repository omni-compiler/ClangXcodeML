#ifndef TYPETABLEVISITOR_H
#define TYPETABLEVISITOR_H

class TypeTableVisitor
    : public XcodeMlVisitorBase<TypeTableVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    bool PreVisitStmt(clang::Stmt *);
    bool PreVisitDecl(clang::Decl *);
};

class TypeTableInfo {
    std::map<std::string, xmlNodePtr> typeNameToNode;
    std::map<xmlNodePtr, std::string> typeNodeToName;
public:
    explicit TypeTableInfo(); // default constructor
    TypeTableInfo(const TypeTableInfo&) = delete;
    TypeTableInfo(TypeTableInfo&&) = delete;
    TypeTableInfo& operator =(const TypeTableInfo &) = delete;
    TypeTableInfo& operator =(const TypeTableInfo &&) = delete;

    std::string getTypeName(xmlNodePtr node);
};

#endif /* !TYPETABLEVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
