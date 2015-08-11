#ifndef TYPETABLEVISITOR_H
#define TYPETABLEVISITOR_H

#include <unordered_map>

namespace std {
    template<>
    struct hash<clang::QualType> {
        size_t operator()(const clang::QualType T) const {
            union union_for_hash {
                size_t value;
                clang::QualType originalT;
                union_for_hash(clang::QualType T) : originalT(T) {};
            };
            const union_for_hash tmp(T);
            return tmp.value;
        }
    };
}

class TypeTableInfo {
    clang::MangleContext *mangleContext;
    std::unordered_map<std::string, clang::QualType> mapFromNameToQualType;
    std::unordered_map<clang::QualType, std::string> mapFromQualTypeToName;
    std::unordered_map<clang::QualType, xmlNodePtr> mapFromQualTypeToXmlNodePtr;

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

    xmlNodePtr createNode(clang::QualType T, const char *fieldname, std::string name);
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

    explicit TypeTableInfo(clang::MangleContext *MC); // default constructor

    std::string registerType(clang::QualType T, xmlNodePtr *curNode = nullptr);
    std::string getTypeName(clang::QualType T);
    void emitAllTypeNode(xmlNodePtr ParentNode);
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
