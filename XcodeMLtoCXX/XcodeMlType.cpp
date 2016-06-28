#include <cassert>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include "XcodeMlType.h"

namespace XcodeMl {

TypeKind typeKind(TypeRef type) {
  return type->kind;
}

TypeRef makeReservedType(std::string name) {
  Type type;
  type.kind = TypeKind::Reserved;
  type.name = name;
  return std::make_shared<Type>(type);
}

TypeRef makePointerType(TypeRef ref) {
  Type type;
  type.kind = TypeKind::Pointer;
  type.type = ref;
  return std::make_shared<Type>(type);
}

TypeRef makeFunctionType(TypeRef returnType, const std::vector<TypeRef>& params) {
  Type type;
  type.kind = TypeKind::Function;
  type.type = returnType;
  type.params = params;
  return std::make_shared<Type>(type);
}

TypeRef makeArrayType(TypeRef elem, size_t size) {
  Type type;
  type.kind = TypeKind::Array;
  type.type = elem;
  type.size = std::make_shared<size_t>(size);
  return std::make_shared<Type>(type);
}

ReservedType getReservedType(TypeRef type) {
  assert(type->kind == TypeKind::Reserved);
  return {type->name};
}

PointerType getPointerType(TypeRef type) {
  assert(type->kind == TypeKind::Pointer);
  return {type->type};
}

FunctionType getFunctionType(TypeRef type) {
  assert(type->kind == TypeKind::Function);
  return {type->type, type->params};
}

ArrayType getArrayType(TypeRef type) {
  assert(type->kind == TypeKind::Array);
  return {type->type, type->size};
}

std::string TypeRefToString(TypeRef type) {
  switch (typeKind(type)) {
    case TypeKind::Reserved:
      return getReservedType(type).name;
    case TypeKind::Pointer:
      return TypeRefToString(getPointerType(type).ref) + "*";
    case TypeKind::Function:
    case TypeKind::Array:
      return "";
  }
}

}
