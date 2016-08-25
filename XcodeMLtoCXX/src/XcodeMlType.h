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
  Type(DataTypeIdent);
  virtual ~Type() = 0;
  friend TypeKind typeKind(TypeRef);
protected:
  virtual TypeKind getKind() = 0;
public:
  virtual std::string makeDeclaration(std::string, const Environment&) = 0;
  DataTypeIdent dataTypeIdent();
private:
  DataTypeIdent ident;
};

class Reserved : public Type {
public:
  Reserved(DataTypeIdent, std::string);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Reserved() override;
private:
  TypeKind getKind() override;
  std::string name;
};

class Pointer : public Type {
public:
  Pointer(DataTypeIdent, TypeRef);
  Pointer(DataTypeIdent, DataTypeIdent);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Pointer() override;
private:
  TypeKind getKind() override;
  DataTypeIdent ref;
};

class Function : public Type {
public:
  using Params = std::vector<std::tuple<DataTypeIdent, std::string>>;
  Function(DataTypeIdent, TypeRef, const std::vector<DataTypeIdent>&);
  Function(DataTypeIdent, TypeRef, const Params&);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Function() override;
private:
  TypeKind getKind() override;
  DataTypeIdent returnValue;
  Params params;
};

class Array : public Type {
public:
  Array(DataTypeIdent, TypeRef, size_t);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Array() override;
private:
  TypeKind getKind() override;
  DataTypeIdent element;
  std::shared_ptr<size_t> size;
};

class Struct : public Type {
public:
  Struct(DataTypeIdent, std::string, std::string, SymbolMap &&);
  std::string makeDeclaration(std::string, const Environment&) override;
  ~Struct() override;
private:
  std::string name;
  std::string tag;
  SymbolMap fields;
  TypeKind getKind() override;
};

TypeRef makeReservedType(DataTypeIdent, std::string);
TypeRef makePointerType(DataTypeIdent, TypeRef);
TypeRef makePointerType(DataTypeIdent, DataTypeIdent);
TypeRef makeFunctionType(DataTypeIdent, TypeRef, const Function::Params&);
TypeRef makeArrayType(DataTypeIdent, TypeRef, size_t);
TypeRef makeStructType(DataTypeIdent, std::string, std::string, SymbolMap &&);

}
#endif /* !XCODEMLTYPE_H */
