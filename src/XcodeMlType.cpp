#include <cassert>
#include "XcodeMlType.h"

XcodeMlTypeKind typeKind(XcodeMlTypeRef type) {
  return type->kind;
}

XcodeMlTypeRef makeReservedType(std::string name) {
  XcodeMlType type;
  type.kind = XcodeMlTypeKind::Reserved;
  type.name = name;
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlTypeRef makePointerType(XcodeMlTypeRef ref) {
  XcodeMlType type;
  type.kind = XcodeMlTypeKind::Pointer;
  type.type = ref;
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlTypeRef makeFunctionType(XcodeMlTypeRef returnType, const std::vector<XcodeMlTypeRef>& params) {
  XcodeMlType type;
  type.kind = XcodeMlTypeKind::Function;
  type.type = returnType;
  type.params = params;
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlTypeRef makeArrayType(XcodeMlTypeRef elem, size_t size) {
  XcodeMlType type;
  type.kind = XcodeMlTypeKind::Array;
  type.type = elem;
  type.size = std::make_shared<size_t>(size);
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlReservedType getReservedType(XcodeMlTypeRef type) {
  assert(type->kind == XcodeMlTypeKind::Reserved);
  return {type->name};
}

XcodeMlPointerType getPointerType(XcodeMlTypeRef type) {
  assert(type->kind == XcodeMlTypeKind::Pointer);
  return {type->type};
}

XcodeMlFunctionType getFunctionType(XcodeMlTypeRef type) {
  assert(type->kind == XcodeMlTypeKind::Function);
  return {type->type, type->params};
}

XcodeMlArrayType getArrayType(XcodeMlTypeRef type) {
  assert(type->kind == XcodeMlTypeKind::Array);
  return {type->type, type->size};
}
