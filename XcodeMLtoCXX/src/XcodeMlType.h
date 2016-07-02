#ifndef XCODEMLTYPE_H
#define XCODEMLTYPE_H

namespace XcodeMl {

class Type;
using TypeRef = std::shared_ptr<Type>; /* not nullable */
/*!
 * \brief A mapping from data type identifiers
 * to actual data types. */
using TypeMap = std::map<std::string, TypeRef>;

struct ReservedType {
  std::string name;
};

struct PointerType {
  TypeRef ref;
};

struct FunctionType {
  TypeRef returnType;
  std::vector<TypeRef> params;
};

struct ArrayType {
  TypeRef elementType;
  std::shared_ptr<size_t> size;
};

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

TypeRef makeReservedType(std::string);
TypeRef makePointerType(TypeRef);
TypeRef makeFunctionType(TypeRef, const std::vector<TypeRef>&);
TypeRef makeArrayType(TypeRef, size_t);

ReservedType getReservedType(TypeRef);
PointerType getPointerType(TypeRef);
FunctionType getFunctionType(TypeRef);
ArrayType getArrayType(TypeRef);
std::string TypeRefToString(TypeRef);

/*!
 * \brief A class that represents data types in XcodeML.
 */
class Type {
public:
  virtual std::string toString() = 0;
  virtual ~Type() = 0;
  friend TypeKind typeKind(TypeRef);
protected:
  virtual TypeKind getKind() = 0;
};

class Reserved : public Type {
public:
  Reserved(std::string);
  std::string toString() override;
  ~Reserved() override;
  friend ReservedType getReservedType(TypeRef);
private:
  TypeKind getKind() override;
  std::string name;
};

class Pointer : public Type {
public:
  Pointer(TypeRef);
  std::string toString() override;
  ~Pointer() override;
  friend PointerType getPointerType(TypeRef);
private:
  TypeKind getKind() override;
  TypeRef ref;
};

class Function : public Type {
public:
  Function(TypeRef, std::vector<TypeRef>);
  std::string toString() override;
  ~Function() override;
  friend FunctionType getFunctionType(TypeRef);
private:
  TypeKind getKind() override;
  TypeRef returnType;
  std::vector<TypeRef> params;
};

class Array : public Type {
public:
  Array(TypeRef, size_t);
  std::string toString() override;
  ~Array() override;
  friend ArrayType getArrayType(TypeRef);
private:
  TypeKind getKind() override;
  TypeRef elementType;
  std::shared_ptr<size_t> size;
};

}
#endif /* !XCODEMLTYPE_H */
