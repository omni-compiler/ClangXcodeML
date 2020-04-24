#ifndef TYPETABLEVISITOR_H
#define TYPETABLEVISITOR_H

#include "llvm/ADT/SmallVector.h"
#include "clang/AST/Mangle.h"

#include "InheritanceInfo.h"
#include <stack>
#include <tuple>
#include <unordered_map>

class NnsTableInfo;

class TypeTableInfo {
  clang::MangleContext *mangleContext;
  InheritanceInfo *inheritanceinfo;
  NnsTableInfo *nnstableinfo;
  std::unordered_map<std::string, clang::QualType> mapFromNameToQualType;
  std::unordered_map<clang::QualType, std::string> mapFromQualTypeToName;
  std::unordered_map<clang::QualType, xmlNodePtr> mapFromQualTypeToXmlNodePtr;
  std::unordered_map<clang::QualType, bool> normalizability;
  std::stack<std::tuple<xmlNodePtr, std::vector<clang::QualType>>>
      typeTableStack;

  int seqForBasicType;
  int seqForPointerType;
  int seqForFunctionType;
  int seqForArrayType;
  int seqForStructType;
  int seqForUnionType;
  int seqForEnumType;
  int seqForClassType;
  int seqForTemplateTypeParmType;
  int seqForInjectedClassNameType;
  int seqForMemberPointerType;
  int seqForDependentNameType;
  int seqForTemplateSpecializationType;
  int seqForOtherType;

  std::unordered_map<clang::QualType, xmlNodePtr> TypeElements;

  bool useLabelType;

  xmlNodePtr createNode(
      clang::QualType T, const char *fieldname, xmlNodePtr traversingNode);
  std::string registerBasicType(clang::QualType T); // "B*"
  std::string registerPointerType(clang::QualType T); // "P*"
  std::string registerFunctionType(clang::QualType T); // "F*"
  std::string registerArrayType(clang::QualType T); // "A*"
  std::string registerRecordType(clang::QualType T); // "S*", "U*", or "C*"
  std::string registerEnumType(clang::QualType T); // "E*"
  std::string registerTemplateTypeParmType(clang::QualType T);
  std::string registerInjectedClassNameType(clang::QualType T);
  std::string registerMemberPointerType(clang::QualType T);
  std::string registerDependentNameType(clang::QualType T);
  std::string registerTemplateSpecializationType(clang::QualType T);
  std::string registerOtherType(clang::QualType T); // "O*"
  void pushType(const clang::QualType &, xmlNodePtr);

public:
  TypeTableInfo() = delete;
  TypeTableInfo(const TypeTableInfo &) = delete;
  TypeTableInfo(TypeTableInfo &&) = delete;
  TypeTableInfo &operator=(const TypeTableInfo &) = delete;
  TypeTableInfo &operator=(const TypeTableInfo &&) = delete;

  explicit TypeTableInfo(clang::MangleContext *MC,
      InheritanceInfo *II,
      NnsTableInfo *NTI); // default constructor

  void registerType(
      clang::QualType T, xmlNodePtr *retNode, xmlNodePtr traversingNode);
  void registerLabelType(void);
  std::string getTypeName(clang::QualType T);
  std::string getTypeNameForLabel(void);
  std::vector<BaseClass> getBaseClasses(clang::QualType type);
  void addInheritance(clang::QualType derived, BaseClass base);
  bool hasBaseClass(clang::QualType type);
  void setNormalizability(clang::QualType, bool);
  bool isNormalizable(clang::QualType);
  void pushTypeTableStack(xmlNodePtr);
  void popTypeTableStack();
  void dump();
};

#endif /* !TYPETABLEVISITOR_H */

///
/// Local Variables:
/// mode: c++
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
