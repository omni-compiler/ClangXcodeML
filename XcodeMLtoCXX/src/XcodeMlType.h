#ifndef XCODEMLTYPE_H
#define XCODEMLTYPE_H

namespace XcodeMl {

class Type;
using TypeRef = std::shared_ptr<Type>; /* not nullable */

enum class TypeKind {
 /*! basic data type (3.4 <basicType> element) */
  Reserved,
  /*! pointer (3.5 <pointerType> element) */
  Pointer,
  /*! function (3.6 <functionType> element) */
  Function,
  /*! C-style array (3.7 <ArrayType> element) */
  Array,
};

TypeKind typeKind(TypeRef);
std::string makeDecl(TypeRef, std::string);

TypeRef makeReservedType(std::string);
TypeRef makePointerType(TypeRef);
TypeRef makeFunctionType(TypeRef, const std::vector<TypeRef>&);
TypeRef makeArrayType(TypeRef, size_t);

std::string TypeRefToString(TypeRef);

/*!
 * \brief A class that represents data types in XcodeML.
 */
class Type {
public:
  virtual ~Type() = 0;
  friend TypeKind typeKind(TypeRef);
  friend std::string makeDecl(TypeRef, std::string);
protected:
  virtual TypeKind getKind() = 0;
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
  TypeRef ref;
};

class Function : public Type {
public:
  Function(TypeRef, const std::vector<TypeRef>&);
  Function(TypeRef, const std::vector<std::tuple<TypeRef,std::string>>&);
  std::string makeDeclaration(std::string) override;
  ~Function() override;
private:
  TypeKind getKind() override;
  TypeRef returnType;
  std::vector<std::tuple<TypeRef,std::string>> params;
};

class Array : public Type {
public:
  Array(TypeRef, size_t);
  std::string makeDeclaration(std::string) override;
  ~Array() override;
private:
  TypeKind getKind() override;
  TypeRef elementType;
  std::shared_ptr<size_t> size;
};

}
#endif /* !XCODEMLTYPE_H */
