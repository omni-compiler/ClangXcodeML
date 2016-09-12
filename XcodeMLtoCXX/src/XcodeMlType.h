#ifndef XCODEMLTYPE_H
#define XCODEMLTYPE_H

namespace XcodeMl {

class Type;
using TypeRef = std::shared_ptr<Type>;

/* data type identifier (3.1 data type identifier) */
using DataTypeIdent = std::string;

class Environment;

enum class TypeKind {
 /*! basic data type (3.4 <basicType> element) */
  Reserved,
  /*! pointer (3.5 <pointerType> element) */
  Pointer,
  /*! function (3.6 <functionType> element) */
  Function,
  /*! C-style array (3.7 <ArrayType> element) */
  Array,
  /*! C-Style struct (3.xx <structType> element) */
  Struct,
};

TypeKind typeKind(TypeRef);
std::string makeDecl(TypeRef, std::string, const Environment&);

std::string TypeRefToString(TypeRef, const Environment& env);

/*!
 * \brief A class that represents data types in XcodeML.
 */
class Type {
public:
  Type(TypeKind, DataTypeIdent, bool = false, bool = false);
  virtual ~Type() = 0;
  virtual Type* clone() const = 0;
  virtual std::string makeDeclaration(std::string, const Environment&) = 0;
  virtual std::string addConstQualifier(std::string) const;
  virtual std::string addVolatileQualifier(std::string) const;
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
  Reserved(DataTypeIdent, std::string);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Reserved() override;
  Type* clone() const override;
  static bool classof(const Type *);
protected:
  Reserved(const Reserved&);
private:
  std::string name;
};

class Pointer : public Type {
public:
  Pointer(DataTypeIdent, TypeRef);
  Pointer(DataTypeIdent, DataTypeIdent);
  std::string makeDeclaration(std::string, const Environment&) override;
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
  using Params = std::vector<std::tuple<DataTypeIdent, std::string>>;
  Function(DataTypeIdent, TypeRef, const std::vector<DataTypeIdent>&);
  Function(DataTypeIdent, TypeRef, const Params&);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Function() override;
  Type* clone() const override;
  static bool classof(const Type *);
protected:
  Function(const Function&);
private:
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
  Array(DataTypeIdent, TypeRef, size_t);
  Array(DataTypeIdent, TypeRef, Size);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Array() override;
  Type* clone() const override;
  std::string addConstQualifier(std::string) const override;
  std::string addVolatileQualifier(std::string) const override;
  static bool classof(const Type *);
protected:
  Array(const Array&);
private:
  DataTypeIdent element;
  Size size;
};

class Struct : public Type {
public:
  class BitSize { /* represents a size of bit-field */
  public:
    BitSize();
    BitSize(size_t);
    bool isValid() const;
    size_t size() const;
  private:
    bool valid;
    size_t size_;
  };

  class Member {
  public:
    Member() = delete;
    Member(const std::string&, const std::string&);
    Member(const Member&) = default;
    ~Member() = default;
    std::string type() const;
    std::string name() const;
  private:
    std::string dataTypeIdent;
    std::string name_;
  };
  using MemberList = std::vector<Member>;

public:
  Struct(const DataTypeIdent&, const std::string&, const MemberList&);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Struct() override;
  Type* clone() const override;
  void setTagName(const std::string&);
  MemberList members() const;
  std::string tagName() const;
  static bool classof(const Type *);
protected:
  Struct(const Struct&);
private:
  std::string tag;
  MemberList fields;
};

TypeRef makeReservedType(DataTypeIdent, std::string, bool = false, bool = false);
TypeRef makePointerType(DataTypeIdent, TypeRef);
TypeRef makePointerType(DataTypeIdent, DataTypeIdent);
TypeRef makeFunctionType(DataTypeIdent, TypeRef, const Function::Params&);
TypeRef makeArrayType(DataTypeIdent, TypeRef, size_t);
TypeRef makeArrayType(DataTypeIdent, TypeRef, Array::Size);
TypeRef makeStructType(const DataTypeIdent&, const std::string&, const Struct::MemberList&);

}
#endif /* !XCODEMLTYPE_H */
