#include <cassert>
#include "XcodeMlType.h"

class XcodeMlType::Impl {
public:
  XcodeMlTypeKind kind;
  union {
    XcodeMlReservedType reservedType;
    XcodeMlPointerType pointerType;
    XcodeMlFunctionType functionType;
    XcodeMlArrayType arrayType;
  };
  Impl& operator=(Impl&& rhs);
  Impl& operator=(Impl const & rhs);
  ~Impl();
};

XcodeMlType::Impl& XcodeMlType::Impl::operator=(Impl&& rhs) {
  switch (rhs.kind) {
    case XcodeMlTypeKind::Reserved:
      reservedType = rhs.reservedType;
      break;
    case XcodeMlTypeKind::Pointer:
      pointerType = rhs.pointerType;
      break;
    case XcodeMlTypeKind::Function:
      functionType = rhs.functionType;
      break;
    case XcodeMlTypeKind::Array:
      arrayType = rhs.arrayType;
      break;
  }
  return *this;
}

XcodeMlType::Impl& XcodeMlType::Impl::operator=(Impl const& rhs) {
  return *this = std::move(rhs);
}

XcodeMlType::Impl::~Impl() {
  switch (kind) {
    case XcodeMlTypeKind::Reserved:
      reservedType.~XcodeMlReservedType();
      break;
    case XcodeMlTypeKind::Pointer:
      pointerType.~XcodeMlPointerType();
      break;
    case XcodeMlTypeKind::Function:
      functionType.~XcodeMlFunctionType();
      break;
    case XcodeMlTypeKind::Array:
      arrayType.~XcodeMlArrayType();
      break;
  }
}

XcodeMlTypeKind typeKind(XcodeMlTypeRef type) {
  return type->impl->kind;
}

XcodeMlTypeRef makeReservedType(std::string name) {
  XcodeMlType type;
  type.impl->kind = XcodeMlTypeKind::Reserved;
  type.impl->reservedType = {name};
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlTypeRef makePointerType(XcodeMlTypeRef ref) {
  XcodeMlType type;
  type.impl->kind = XcodeMlTypeKind::Pointer;
  type.impl->pointerType = {ref};
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlTypeRef makeFunctionType(XcodeMlTypeRef returnType, const std::vector<XcodeMlTypeRef>& params) {
  XcodeMlType type;
  type.impl->kind = XcodeMlTypeKind::Function;
  type.impl->functionType = {returnType, params};
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlTypeRef makeArrayType(XcodeMlTypeRef elem, size_t size) {
  XcodeMlType type;
  type.impl->kind = XcodeMlTypeKind::Array;
  type.impl->arrayType = {elem, std::make_shared<size_t>(size)};
  return std::make_shared<XcodeMlType>(type);
}

XcodeMlReservedType getReservedType(XcodeMlTypeRef type) {
  assert(type->impl->kind == XcodeMlTypeKind::Reserved);
  return type->impl->reservedType;
}

XcodeMlPointerType getPointerType(XcodeMlTypeRef type) {
  assert(type->impl->kind == XcodeMlTypeKind::Pointer);
  return type->impl->pointerType;
}

XcodeMlFunctionType getFunctionType(XcodeMlTypeRef type) {
  assert(type->impl->kind == XcodeMlTypeKind::Function);
  return type->impl->functionType;
}

XcodeMlArrayType getArrayType(XcodeMlTypeRef type) {
  assert(type->impl->kind == XcodeMlTypeKind::Array);
  return type->impl->arrayType;
}
