#ifndef TYPETABLEVISITOR_H
#define TYPETABLEVISITOR_H

#include "InheritanceInfo.h"
#include <unordered_map>

class TypeTableInfo {
    clang::MangleContext *mangleContext;
    std::unordered_map<std::string, clang::QualType> mapFromNameToQualType;
    std::unordered_map<clang::QualType, std::string> mapFromQualTypeToName;
    std::unordered_map<clang::QualType, xmlNodePtr> mapFromQualTypeToXmlNodePtr;
    InheritanceInfo *inheritanceinfo;

    int seqForBasicType;
    int seqForPointerType;
    int seqForFunctionType;
    int seqForArrayType;
    int seqForStructType;
    int seqForUnionType;
    int seqForEnumType;
    int seqForClassType;
    int seqForOtherType;

    std::vector<xmlNodePtr> basicTypeNodes;
    std::vector<xmlNodePtr> pointerTypeNodes;
    std::vector<xmlNodePtr> functionTypeNodes;
    std::vector<xmlNodePtr> arrayTypeNodes;
    std::vector<xmlNodePtr> structTypeNodes;
    std::vector<xmlNodePtr> unionTypeNodes;
    std::vector<xmlNodePtr> enumTypeNodes;
    std::vector<xmlNodePtr> classTypeNodes;
    std::vector<xmlNodePtr> otherTypeNodes;
    bool useLabelType;

    xmlNodePtr createNode(clang::QualType T, const char *fieldname,
                          xmlNodePtr traversingNode);
    std::string registerBasicType(clang::QualType T); // "B*"
    std::string registerPointerType(clang::QualType T); // "P*"
    std::string registerFunctionType(clang::QualType T); // "F*"
    std::string registerArrayType(clang::QualType T); // "A*"
    std::string registerRecordType(clang::QualType T); // "S*", "U*", or "C*"
    std::string registerEnumType(clang::QualType T); // "E*"
    std::string registerOtherType(clang::QualType T); // "O*"

public:
    TypeTableInfo() = delete;
    TypeTableInfo(const TypeTableInfo&) = delete;
    TypeTableInfo(TypeTableInfo&&) = delete;
    TypeTableInfo& operator =(const TypeTableInfo &) = delete;
    TypeTableInfo& operator =(const TypeTableInfo &&) = delete;

    explicit TypeTableInfo(clang::MangleContext *MC, InheritanceInfo *II); // default constructor

    void registerType(clang::QualType T, xmlNodePtr *retNode,
                             xmlNodePtr traversingNode);
    void registerLabelType(void);
    std::string getTypeName(clang::QualType T);
    std::string getTypeNameForLabel(void);
    void emitAllTypeNode(xmlNodePtr ParentNode);
    std::vector<clang::QualType> getBaseClasses(clang::QualType type);
    void addInheritance(clang::QualType derived, clang::QualType base);
};

class TypeTableVisitor
    : public XcodeMlVisitorBase<TypeTableVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override;
    bool PreVisitStmt(clang::Stmt *);
    bool PreVisitDecl(clang::Decl *);
    bool PreVisitType(clang::QualType);
    bool PreVisitAttr(clang::Attr *);
    bool PreVisitNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc);
    bool PreVisitTypeLoc(clang::TypeLoc);
    bool PreVisitDeclarationNameInfo(clang::DeclarationNameInfo);
    bool FullTrace(void) const;
};

#endif /* !TYPETABLEVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
