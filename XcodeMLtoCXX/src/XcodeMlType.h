#ifndef XCODEMLTYPE_H
#define XCODEMLTYPE_H

namespace XcodeMl {

class Type;
using TypeRef = std::shared_ptr<Type>; /* not nullable */

/* data type identifier (3.1 data type identifier) */
using DataTypeIdent = std::string;

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
std::string makeDecl(TypeRef, std::string);

std::string TypeRefToString(TypeRef);

/*!
 * \brief A class that represents data types in XcodeML.
 */
class Type {
public:
  virtual ~Type() = 0;
  friend TypeKind typeKind(TypeRef);
protected:
  virtual TypeKind getKind() = 0;
public:
  virtual std::string makeDeclaration(std::string) = 0;
};

class Reserved : public Type {
public:
  Reserved(std::string);
  std::string makeDeclaration(std::string) override;
  ~Reserved() override;
private:
  TypeKind getKind() override;
  std::string name;
};

class Pointer : public Type {
public:
  Pointer(TypeRef);
  std::string makeDeclaration(std::string) override;
  ~Pointer() override;
private:
  TypeKind getKind() override;
  DataTypeIdent ref;
};

class Function : public Type {
public:
  Function(TypeRef, const std::vector<TypeRef>&);
  Function(TypeRef, const std::vector<std::tuple<TypeRef,std::string>>&);
  std::string makeDeclaration(std::string) override;
  ~Function() override;
private:
  TypeKind getKind() override;
  DataTypeIdent returnType;
  std::vector<std::tuple<DataTypeIdent,std::string>> params;
};

class Array : public Type {
public:
  Array(TypeRef, size_t);
  std::string makeDeclaration(std::string) override;
  ~Array() override;
private:
  TypeKind getKind() override;
  DataTypeIdent elementType;
  std::shared_ptr<size_t> size;
};

class Struct : public Type {
public:
  Struct(std::string, std::string, SymbolMap &&);
  std::string makeDeclaration(std::string) override;
  ~Struct() override;
private:
  std::string name;
  std::string tag;
  SymbolMap fields;
  TypeKind getKind() override;
};

}
#endif /* !XCODEMLTYPE_H */
