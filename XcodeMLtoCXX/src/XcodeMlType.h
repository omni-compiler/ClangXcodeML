#ifndef XCODEMLTYPE_H
#define XCODEMLTYPE_H

namespace XcodeMl {

class Type;
using TypeRef = std::shared_ptr<Type>;

/* data type identifier (3.1 data type identifier) */
using DataTypeIdent = std::string;

using CodeFragment = CXXCodeGen::StringTreeRef;

class Environment;

class MemberDecl {
public:
  MemberDecl(const DataTypeIdent&, const CodeFragment&);
  MemberDecl(const DataTypeIdent&, const CodeFragment&, size_t);
  CodeFragment makeDeclaration(const Environment&) const;
private:
  DataTypeIdent dtident;
  CodeFragment name;
  llvm::Optional<size_t> bitfield;
};

enum class TypeKind {
 /*! Built-in type */
  Reserved,
 /*! basic data type (3.4 <basicType> element) */
  Qualified,
  /*! pointer (3.5 <pointerType> element) */
  Pointer,
  /*! function (3.6 <functionType> element) */
  Function,
  /*! C-style array (3.7 <ArrayType> element) */
  Array,
  /*! C-Style struct (3.xx <structType> element) */
  Struct,
  /*! C-style enum type */
  Enum,
  /*! C-style union type */
  Union,
  /*! C++-style class */
  Class,
  /*! Other type */
  Other,
};

TypeKind typeKind(TypeRef);
CodeFragment makeDecl(TypeRef, CodeFragment, const Environment&);

CodeFragment TypeRefToString(TypeRef, const Environment& env);

/*!
 * \brief A class that represents data types in XcodeML.
 */
class Type {
public:
  Type(TypeKind, DataTypeIdent, bool = false, bool = false);
  virtual ~Type() = 0;
  virtual Type* clone() const = 0;
  virtual CodeFragment makeDeclaration(CodeFragment, const Environment&) = 0;
  virtual CodeFragment addConstQualifier(CodeFragment) const;
  virtual CodeFragment addVolatileQualifier(CodeFragment) const;
  bool isConst() const;
  bool isVolatile() const;
  void setConst(bool);
  void setVolatile(bool);
  DataTypeIdent dataTypeIdent();
  TypeKind getKind() const;
protected:
  Type(const Type&);
private:
  TypeKind kind;
  DataTypeIdent ident;
  bool constness;
  bool volatility;
};

class Reserved : public Type {
public:
  Reserved(DataTypeIdent, CodeFragment);
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  ~Reserved() override;
  Type* clone() const override;
  static bool classof(const Type *);
protected:
  Reserved(const Reserved&);
private:
  CodeFragment name;
};

class QualifiedType : public Type {
public:
  QualifiedType(DataTypeIdent, DataTypeIdent, bool, bool);
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  ~QualifiedType() override;
  Type* clone() const override;
  static bool classof(const Type*);
protected:
  QualifiedType(const QualifiedType&);
private:
  DataTypeIdent underlying;
  bool isConst;
  bool isVolatile;
};

class Pointer : public Type {
public:
  Pointer(DataTypeIdent, TypeRef);
  Pointer(DataTypeIdent, DataTypeIdent);
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  ~Pointer() override;
  Type* clone() const override;
  static bool classof(const Type *);
protected:
  Pointer(const Pointer&);
private:
  DataTypeIdent ref;
};

class Function : public Type {
public:
  using Params = std::vector<std::tuple<DataTypeIdent, CodeFragment>>;
  Function(DataTypeIdent, TypeRef, const std::vector<DataTypeIdent>&);
  Function(DataTypeIdent, TypeRef, const Params&);
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  CodeFragment makeDeclaration(CodeFragment, const std::vector<CodeFragment>&, const Environment&);
  ~Function() override;
  Type* clone() const override;
  static bool classof(const Type *);
protected:
  Function(const Function&);
private:
  bool isParamListEmpty() const;

  DataTypeIdent returnValue;
  Params params;
};

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
  Array(DataTypeIdent, DataTypeIdent, Size);
  Array(DataTypeIdent, DataTypeIdent, size_t);
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  ~Array() override;
  Type* clone() const override;
  CodeFragment addConstQualifier(CodeFragment) const override;
  CodeFragment addVolatileQualifier(CodeFragment) const override;
  static bool classof(const Type *);
protected:
  Array(const Array&);
private:
  DataTypeIdent element;
  Size size;
};

class Struct : public Type {
public:
  using MemberList = std::vector<MemberDecl>;

public:
  Struct(const DataTypeIdent&, const CodeFragment&, const MemberList&);
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  CodeFragment makeStructDefinition(const Environment&) const;
  ~Struct() override;
  Type* clone() const override;
  void setTagName(const CodeFragment&);
  MemberList members() const;
  CodeFragment tagName() const;
  static bool classof(const Type *);
protected:
  Struct(const Struct&);
private:
  CodeFragment tag;
  MemberList fields;
};

class EnumType : public Type {
public:
  using EnumName = llvm::Optional<CodeFragment>;
  EnumType(const DataTypeIdent&, const EnumName&);
  EnumType(const DataTypeIdent&, const EnumName&, const CodeFragment&);
  ~EnumType() override = default;
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  Type* clone() const override;
  static bool classof(const Type *);
  void setName(const std::string&);
protected:
  EnumType(const EnumType&);
private:
  EnumName name_;
  CodeFragment declBody;
};

class UnionType : public Type {
public:
  using UnionName = llvm::Optional<CodeFragment>;
  UnionType(const DataTypeIdent&, const UnionName&);
  UnionType(const DataTypeIdent&, const UnionName&, const std::vector<MemberDecl>&);
  ~UnionType() override = default;
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  Type* clone() const override;
  static bool classof(const Type *);
  void setName(const std::string&);
protected:
  UnionType(const UnionType&);
private:
  UnionName name_;
  std::vector<MemberDecl> members;
};

enum class AccessSpec {
  Public,
  Private,
  Protected,
};

std::string string_of_accessSpec(AccessSpec);
AccessSpec accessSpec_of_string(const std::string&);

class ClassType : public Type {
public:
  using ClassName = llvm::Optional<CodeFragment>;
  using MemberName = std::string;
  using Symbols = std::vector<std::tuple<MemberName, DataTypeIdent>>;
  ClassType(const DataTypeIdent&, const CodeFragment&, const Symbols&);
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  ~ClassType() override = default;
  Type* clone() const override;
  ClassName name() const;
  void setName(const std::string&);
  Symbols getSymbols() const;
  static bool classof(const Type *);
protected:
  ClassType(const ClassType&);
private:
  ClassName name_;
  Symbols classScopeSymbols;
};

class OtherType : public Type {
public:
  OtherType(const DataTypeIdent&);
  ~OtherType() override = default;
  CodeFragment makeDeclaration(CodeFragment, const Environment&) override;
  Type* clone() const override;
  static bool classof(const Type *);
protected:
  OtherType(const OtherType&);
};

TypeRef makeReservedType(DataTypeIdent, CodeFragment, bool = false, bool = false);
TypeRef makeQualifiedType(const DataTypeIdent&, const DataTypeIdent&, bool, bool);
TypeRef makePointerType(DataTypeIdent, TypeRef);
TypeRef makePointerType(DataTypeIdent, DataTypeIdent);
TypeRef makeFunctionType(DataTypeIdent, TypeRef, const Function::Params&);
TypeRef makeArrayType(DataTypeIdent, TypeRef, size_t);
TypeRef makeArrayType(DataTypeIdent, TypeRef, size_t);
TypeRef makeArrayType(DataTypeIdent, TypeRef, Array::Size);
TypeRef makeArrayType(DataTypeIdent, DataTypeIdent, Array::Size);
TypeRef makeArrayType(DataTypeIdent, DataTypeIdent, size_t);
TypeRef makeEnumType(const DataTypeIdent&);
TypeRef makeClassType(const DataTypeIdent&, const ClassType::Symbols&);
TypeRef makeStructType(const DataTypeIdent&, const CodeFragment&, const Struct::MemberList&);
TypeRef makeOtherType(const DataTypeIdent&);

}
#endif /* !XCODEMLTYPE_H */
