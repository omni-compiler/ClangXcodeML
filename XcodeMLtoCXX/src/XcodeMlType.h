#ifndef XCODEMLTYPE_H
#define XCODEMLTYPE_H
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

namespace XcodeMl {

class Type;
using TypeRef = std::shared_ptr<Type>;

/*! XcodeML data type identifier (3.1 data type identifier) */
using DataTypeIdent = std::string;

/*! Represents a string that may appear in C++ source code. */
using CodeFragment = CXXCodeGen::StringTreeRef;

class TypeTable;
class UnqualId;

/*!
 * \brief Represents a member declaration in C-style struct or union.
 */
class MemberDecl {
public:
  MemberDecl(const DataTypeIdent &, const CodeFragment &);
  MemberDecl(const DataTypeIdent &, const CodeFragment &, size_t);
  CodeFragment makeDeclaration(const TypeTable &, const NnsTable &) const;

private:
  DataTypeIdent dtident;
  CodeFragment name;
  llvm::Optional<size_t> bitfield;
};

enum class TypeKind {
  /*! Built-in type */
  Reserved,
  /*! XcodeML basic data type (3.4 <basicType> element) */
  Qualified,
  /*! Pointer type (3.5 <pointerType> element) */
  Pointer,
  /*! Pointer to member */
  MemberPointer,
  /*! Lvalue reference type */
  LValueReference,
  /*! Rvalue reference type */
  RValueReference,
  /*! Function type (3.6 <functionType> element) */
  Function,
  /*! C-style array type (3.7 <ArrayType> element) */
  Array,
  /*! C-Style struct type (3.xx <structType> element) */
  Struct,
  /*! C-style enum type */
  Enum,
  /*! C-style union type */
  Union,
  /*! (C++-style) class type */
  Class,
  /*! Template type parameter */
  TemplateTypeParm,
  /*! Template Specialization */
  TemplateSpecialization,
  DependentName,
  DeclType,
  DependentTemplateSpecialization,
  PackExpansion,
  Atomic,
  UnaryTransform,
  /*! Other type */
  Other,
};

TypeKind typeKind(TypeRef);
CodeFragment makeDecl(
    TypeRef, CodeFragment, const TypeTable &, const NnsTable &);

/*!
 * \brief Returns a code fragment string that represents the given data type
 * e.g. `int (*)(const double&)`.
 */
CodeFragment TypeRefToString(
    TypeRef, const TypeTable &env, const NnsTable &nnsTable);

/*!
 * \brief A class that represents data types in XcodeML.
 */
class Type {
public:
  Type(TypeKind, DataTypeIdent, bool = false, bool = false);
  virtual ~Type() = 0;
  virtual Type *clone() const = 0;
  virtual CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) = 0;

  /*!
   * \brief Return a code fragment string created by adding the `const`
   * qualifier
   * to the given declarator.
   *
   * Typically, this function adds the prefix "const" to the beginning of the
   * given
   * string (`x` -> `const x`). Some derived classes use different ways. For
   * example,
   * `XcodeMl::Array::addConstQualifier` returns a string identical to the
   * parameter.
   */
  virtual CodeFragment addConstQualifier(CodeFragment) const;

  /*!
   * \brief Return a code fragment string created by adding the `volatile`
   * qualifier to the given declarator.
   *
   * Typically, this function adds the prefix "volatile" to the beginning of
   * the
   * given string (`x` -> `volatile x`). Some derived classes use different
   * ways.
   * For example, `XcodeMl::Array::addVolatileQualifier` returns a string
   * identical to the parameter.
   */
  virtual CodeFragment addVolatileQualifier(CodeFragment) const;
  bool isConst() const;
  bool isVolatile() const;
  void setConst(bool);
  void setVolatile(bool);
  DataTypeIdent dataTypeIdent();
  TypeKind getKind() const;

protected:
  Type(const Type &);

private:
  TypeKind kind;
  DataTypeIdent ident;
  bool constness;
  bool volatility;
};

/*!
 * \brief Represents built-in data type e.g. `int`.
 *
 * A type name may contain whitespaces (`unsigned short`)
 * while XcodeML data type identifier can't (`unsigned_short`).
 */
class Reserved : public Type {
public:
  Reserved(DataTypeIdent, CodeFragment);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~Reserved() override;
  Type *clone() const override;
  static bool classof(const Type *);

protected:
  Reserved(const Reserved &);

private:
  CodeFragment name;
};

/*!
 * \brief Represents a cv-qualified type.
 *
 * This class was introduced in March 2017 in order to implement
 * XcodeML `basicType` element.
 */
class QualifiedType : public Type {
public:
  QualifiedType(DataTypeIdent dtident,
      DataTypeIdent underlyingType,
      bool isConst,
      bool isVolatile);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~QualifiedType() override;
  Type *clone() const override;
  static bool classof(const Type *);

protected:
  QualifiedType(const QualifiedType &);

private:
  DataTypeIdent underlying;
  bool isConst;
  bool isVolatile;
};

class Pointer : public Type {
public:
  Pointer(DataTypeIdent dtident, TypeRef pointee);
  Pointer(DataTypeIdent dtident, DataTypeIdent pointee);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~Pointer() override;
  Type *clone() const override;
  static bool classof(const Type *);
  TypeRef getPointee(const TypeTable &) const;

protected:
  Pointer(const Pointer &);

private:
  DataTypeIdent ref;
};

class MemberPointer : public Type {
public:
  MemberPointer(
      DataTypeIdent dtident, DataTypeIdent pointee, DataTypeIdent record);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~MemberPointer() override = default;
  Type *clone() const override;
  static bool classof(const Type *T);

protected:
  MemberPointer(const MemberPointer &) = default;

private:
  DataTypeIdent pointee;
  DataTypeIdent record;
};

/*! \brief Represents an lvalue or rvalue reference type. */
class ReferenceType : public Type {
public:
  ReferenceType(const DataTypeIdent &dtident,
      TypeKind kind,
      const DataTypeIdent &pointee);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override = 0;
  ~ReferenceType() override = 0;
  Type *clone() const override = 0;
  TypeRef getPointee(const TypeTable &) const;

protected:
  ReferenceType(const ReferenceType &) = default;
  DataTypeIdent ref;
};

class LValueReferenceType : public ReferenceType {
public:
  LValueReferenceType(
      const DataTypeIdent &dtident, const DataTypeIdent &pointee);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~LValueReferenceType() override = default;
  Type *clone() const override;
  static bool classof(const Type *);

protected:
  LValueReferenceType(const LValueReferenceType &) = default;
};

class RValueReferenceType : public ReferenceType {
public:
  RValueReferenceType(
      const DataTypeIdent &dtident, const DataTypeIdent &pointee);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~RValueReferenceType() override = default;
  Type *clone() const override;
  static bool classof(const Type *);

protected:
  RValueReferenceType(const RValueReferenceType &) = default;
};

class ParamList {
public:
  ParamList() = default;
  ParamList &operator=(const ParamList &) = default;
  ParamList(const std::vector<DataTypeIdent> &paramTypes, bool ellipsis);
  bool isVariadic() const;
  bool isEmpty() const;
  CodeFragment makeDeclaration(const std::vector<CodeFragment> &paramNames,
      const TypeTable &typeTable,
      const NnsTable &nnsTable) const;

private:
  std::vector<DataTypeIdent> dtidents;
  bool hasEllipsis;
};

class Function : public Type {
public:
  Function(DataTypeIdent,
      const DataTypeIdent &dtident,
      const std::vector<DataTypeIdent> &paramTypes,
      bool isVariadic = false);
  CodeFragment makeDeclarationWithoutReturnType(CodeFragment funcName,
      const std::vector<CodeFragment> &argNames,
      const TypeTable &env,
      const NnsTable &nnsTable);
  CodeFragment makeDeclarationWithoutReturnType(
      CodeFragment funcName, const TypeTable &env, const NnsTable &nnsTable);
  CodeFragment makeDeclaration(
      CodeFragment funcName, const TypeTable &env, const NnsTable &) override;
  CodeFragment makeDeclaration(CodeFragment funcName,
      const std::vector<CodeFragment> &argNames,
      const TypeTable &env,
      const NnsTable &nnsTable);
  virtual CodeFragment addConstQualifier(CodeFragment) const override;
  virtual CodeFragment addVolatileQualifier(CodeFragment) const override;
  std::vector<CodeFragment> argNames() const;
  ~Function() override;
  Type *clone() const override;
  static bool classof(const Type *);

protected:
  Function(const Function &);

private:
  bool isParamListEmpty() const;

  DataTypeIdent returnValue;
  ParamList params;
  std::vector<CodeFragment> defaultArgs;
};

/*! \brief Represents C/C++ array. */
class Array : public Type {
public:
  struct Size {
    enum class Kind {
      Integer,
      Variable,
      /* Expression, // FIXME: Unimplemented */
    };
    Kind kind;
    size_t size;
    static Size makeIntegerSize(size_t);
    static Size makeVariableSize();

  private:
    Size(Kind, size_t);
  };

public:
  Array(DataTypeIdent dtident, DataTypeIdent element, Size size);
  Array(DataTypeIdent dtident, DataTypeIdent element, size_t size);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~Array() override;
  Type *clone() const override;
  CodeFragment addConstQualifier(CodeFragment) const override;
  CodeFragment addVolatileQualifier(CodeFragment) const override;
  static bool classof(const Type *);
  CodeFragment makeVsizeDeclaration(
     CodeFragment, const TypeTable &, const NnsTable &  );

  /*! Returns the element type as `XcodeMl::TypeRef`. */
  TypeRef getElemType(const TypeTable &) const;
  bool isFixedSize(){ return (size.kind == Size::Kind::Integer);}
protected:
  Array(const Array &);

private:
  DataTypeIdent element;
  Size size;
};

class Struct : public Type {
public:
  using MemberList = std::vector<MemberDecl>;

public:
  Struct(const DataTypeIdent &, const CodeFragment &, const MemberList &);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  CodeFragment makeStructDefinition(const TypeTable &, const NnsTable &) const;
  ~Struct() override;
  Type *clone() const override;
  void setTagName(const CodeFragment &);
  MemberList members() const;
  CodeFragment tagName() const;
  static bool classof(const Type *);

protected:
  Struct(const Struct &);

private:
  CodeFragment tag;
  MemberList fields;
};

class EnumType : public Type {
public:
  using EnumName = std::shared_ptr<UnqualId>; // nullable
  EnumType(const DataTypeIdent &, const EnumName &);
  ~EnumType() override = default;
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
  EnumName name() const;
  void setName(const std::string &);

protected:
  EnumType(const EnumType &);

private:
  EnumName name_;
};

class UnionType : public Type {
public:
  using UnionName = llvm::Optional<CodeFragment>;
  UnionType(const DataTypeIdent &, const UnionName &);
  UnionType(const DataTypeIdent &,
      const UnionName &,
      const std::vector<MemberDecl> &);
  ~UnionType() override = default;
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
  void setName(const std::string &);

protected:
  UnionType(const UnionType &);

private:
  UnionName name_;
  std::vector<MemberDecl> members;
};

/*! \brief Represents access-specifier. */
enum class AccessSpec {
  Public,
  Private,
  Protected,
};

std::string string_of_accessSpec(AccessSpec);
AccessSpec accessSpec_of_string(const std::string &);

/*! \brief Represents class-key (`class`, `struct`, or `union`). */
enum class CXXClassKind {
  Class,
  Struct,
  Union,
};
struct TemplateArg {
  int argType;
  DataTypeIdent ident;
};
using TemplateArgList = std::vector<TemplateArg>;

/*!
 * \brief Converts `XcodeMl::CXXClassKind` to string (`"class"`,
 * `"struct"`, or `"union"`)
 */
std::string getClassKey(CXXClassKind kind);
/*! \brief Represents (C++-style) class. */
class ClassType : public Type {
public:
  using ClassName = CodeFragment;
  using MemberName = std::shared_ptr<UnqualId>;
  using Symbols = std::vector<std::tuple<MemberName, DataTypeIdent>>;
  using BaseClass = std::tuple<std::string, DataTypeIdent, bool>;
  //using TemplateArg = DataTypeIdent;
  ClassType(const DataTypeIdent &, const CodeFragment &, const Symbols &);
  ClassType(const DataTypeIdent &,
      CXXClassKind,
      const llvm::Optional<std::string> &nns,
      const CodeFragment &,
      const std::vector<BaseClass> &,
      const Symbols &,
	    const llvm::Optional<TemplateArgList> &,
	    uintptr_t node ) ;
  ClassType(const DataTypeIdent &, const Symbols &);
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  ~ClassType() override = default;
  Type *clone() const override;
  CXXClassKind classKind() const;
  ClassName name() const;
  void setName(const std::string &);
  void setName(const CodeFragment &);
  Symbols getSymbols() const;
  std::vector<BaseClass> getBases() const;
  bool isClassTemplateSpecialization() const;
  llvm::Optional<CodeFragment> getAsTemplateId(
      const TypeTable &typeTable, const NnsTable &nnsTable) const;
  static bool classof(const Type *);
  xmlNodePtr getNode(){ return reinterpret_cast<xmlNodePtr>(node);};
protected:
  ClassType(const ClassType &);

private:
  CXXClassKind classKind_;
  llvm::Optional<std::string> nnsident;
  ClassName name_;
  std::vector<BaseClass> bases_;
  Symbols classScopeSymbols;
  llvm::Optional<TemplateArgList> templateArgs;
  uintptr_t node;
};

class TemplateTypeParm : public Type {
public:
  TemplateTypeParm(const DataTypeIdent &dtident, const CodeFragment &name, int pack);
  ~TemplateTypeParm() override = default;
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
  void setSpelling(CodeFragment);
  llvm::Optional<CodeFragment> getSpelling() const;
  bool isPack(){return pack;}
protected:
  TemplateTypeParm(const TemplateTypeParm &);

private:
  bool pack;
  llvm::Optional<CodeFragment> pSpelling;
};
class TemplateSpecializationType : public Type {
  CodeFragment name;
  llvm::Optional<TemplateArgList> templateArgs;
public:
  TemplateSpecializationType(const DataTypeIdent &, const CodeFragment &,const llvm::Optional<TemplateArgList> &);
  ~TemplateSpecializationType() override = default;
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
  llvm::Optional<CodeFragment> getSpelling() const;

};
class DependentNameType: public Type {
  const DataTypeIdent upper;
  const DataTypeIdent member;

public:
  DependentNameType(const DataTypeIdent &, const DataTypeIdent &, const DataTypeIdent &);
  ~DependentNameType() override = default;
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
protected:
  DependentNameType(const DependentNameType &);
};

class DeclType : public Type{
public:
    DeclType(const DataTypeIdent &);
    ~DeclType() override = default;
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);

protected:
  DeclType(const DeclType &);

};

class PackExpansionType: public Type{
  const DataTypeIdent pattern;
public:
  PackExpansionType(const DataTypeIdent &, const DataTypeIdent &);
  ~PackExpansionType() override = default;
  CodeFragment makeDeclaration
  (CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
protected:
  PackExpansionType(const PackExpansionType&);
};

class UnaryTransformType : public Type {
  const DataTypeIdent utype;
public:
  UnaryTransformType(const DataTypeIdent &, const DataTypeIdent &);
  ~UnaryTransformType() override = default;
  CodeFragment makeDeclaration
  (CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
protected:
  UnaryTransformType(const UnaryTransformType &);
};

class AtomicType: public Type{
  const DataTypeIdent valuetype;
public:
  AtomicType(const DataTypeIdent &, const DataTypeIdent &);
  ~AtomicType() override = default;
  CodeFragment makeDeclaration
  (CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
protected:
  AtomicType(const AtomicType &);
};
class DependentTemplateSpecializationType: public Type{
public:
  DependentTemplateSpecializationType(const DataTypeIdent&);
  ~DependentTemplateSpecializationType() override = default;
  CodeFragment makeDeclaration
  (CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);
};
class OtherType : public Type {
public:
  OtherType(const DataTypeIdent &);
  ~OtherType() override = default;
  CodeFragment makeDeclaration(
      CodeFragment, const TypeTable &, const NnsTable &) override;
  Type *clone() const override;
  static bool classof(const Type *);

protected:
  OtherType(const OtherType &);
};

TypeRef makeReservedType(
    DataTypeIdent, CodeFragment, bool = false, bool = false);
TypeRef makeQualifiedType(const DataTypeIdent &ident,
    const DataTypeIdent &underlyingType,
    bool isConst,
    bool isVolatile);
TypeRef makePointerType(DataTypeIdent, TypeRef);
TypeRef makePointerType(DataTypeIdent, DataTypeIdent);
TypeRef makeMemberPointerType(
    DataTypeIdent dtident, DataTypeIdent pointee, DataTypeIdent record);
TypeRef makeLValueReferenceType(const DataTypeIdent &, const DataTypeIdent &);
TypeRef makeRValueReferenceType(const DataTypeIdent &, const DataTypeIdent &);
TypeRef makeArrayType(DataTypeIdent, TypeRef, size_t);
TypeRef makeArrayType(DataTypeIdent, TypeRef, size_t);
TypeRef makeArrayType(DataTypeIdent, TypeRef, Array::Size);
TypeRef makeArrayType(DataTypeIdent, DataTypeIdent, Array::Size);
TypeRef makeArrayType(DataTypeIdent, DataTypeIdent, size_t);
TypeRef makeEnumType(
    const DataTypeIdent &, const std::shared_ptr<XcodeMl::UnqualId> tagname);
TypeRef makeClassType(const DataTypeIdent &, const ClassType::Symbols &);
TypeRef makeClassType(const DataTypeIdent &dtident,
    const llvm::Optional<std::string> nnsident,
    const CodeFragment &className,
    const std::vector<ClassType::BaseClass> &bases,
    const ClassType::Symbols &members,
		      const llvm::Optional<TemplateArgList> &templateArgs,
		      const xmlNodePtr node);
TypeRef makeCXXUnionType(const DataTypeIdent &ident,
    const llvm::Optional<std::string> nnsident,
    const CodeFragment &unionName,
    const std::vector<ClassType::BaseClass> &bases,
    const ClassType::Symbols &members,
			 const llvm::Optional<TemplateArgList> &templateArgs,
			 const xmlNodePtr node);
TypeRef makeFunctionType(const DataTypeIdent &ident,
    const DataTypeIdent &returnType,
    const std::vector<DataTypeIdent> &paramTypes);
TypeRef makeFunctionType(const DataTypeIdent &ident,
    const TypeRef &returnType,
    const std::vector<DataTypeIdent> &paramTypes);
TypeRef makeStructType(
    const DataTypeIdent &, const CodeFragment &, const Struct::MemberList &);
  TypeRef makeTemplateTypeParm(const DataTypeIdent &, const CodeFragment &, int);
TypeRef makeVariadicFunctionType(const DataTypeIdent &ident,
    const DataTypeIdent &returnType,
    const std::vector<DataTypeIdent> &paramTypes);
TypeRef makeTemplateSpecializationType(const DataTypeIdent& ,
				       const CodeFragment &,
				       const llvm::Optional<TemplateArgList> &
				       );
TypeRef makeOtherType(const DataTypeIdent &);
TypeRef makeDeclType(const DataTypeIdent &);
TypeRef makeDependentNameType(const DataTypeIdent &, const DataTypeIdent &, const DataTypeIdent &);
TypeRef makeDependentTemplateSpecializationType(const DataTypeIdent & );
TypeRef makeAtomicType(const DataTypeIdent &, const DataTypeIdent &);
TypeRef makePackExpansionType(const DataTypeIdent &, const DataTypeIdent &);
TypeRef makeUnaryTransformType(const DataTypeIdent &, const DataTypeIdent &);
bool hasParen(const TypeRef &, const TypeTable &);
}
#endif /* !XCODEMLTYPE_H */
