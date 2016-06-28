#ifndef XCODEMLTYPE_H
#define XCODEMLTYPE_H

namespace XcodeMl {

class Type;
using TypeRef = std::shared_ptr<Type>; /* not nullable */
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
  Reserved,
  Pointer,
  Function,
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

class Type {
public:
  friend TypeKind typeKind(TypeRef);

  friend TypeRef XcodeMl::makeReservedType(std::string);
  friend TypeRef XcodeMl::makePointerType(TypeRef);
  friend TypeRef XcodeMl::makeFunctionType(TypeRef, const std::vector<TypeRef>&);
  friend TypeRef XcodeMl::makeArrayType(TypeRef, size_t);

  friend ReservedType XcodeMl::getReservedType(TypeRef);
  friend PointerType XcodeMl::getPointerType(TypeRef);
  friend FunctionType XcodeMl::getFunctionType(TypeRef);
  friend ArrayType XcodeMl::getArrayType(TypeRef);
  friend std::string XcodeMl::TypeRefToString(TypeRef);
private:
  TypeKind kind;
  /* FIXME: knows too much */
  std::string name;
  TypeRef type;
  std::vector<TypeRef> params;
  std::shared_ptr<size_t> size;
};

}

#endif /* !XCODEMLTYPE_H */
