#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "XcodeMlType.h"

namespace XcodeMl {

Reserved::ReservedType(std::string dataType):
  name(dataType)
{}

std::string ReservedType::toString() {
  return name;
}

TypeKind Reserved::getKind() {
  return TypeKind::Reserved;
}

Reserved::~Reserved() = default;

Pointer::Pointer(TypeRef signified):
  ref(signified)
{}

std::string Pointer::toString() {
  assert(ref);
  return ref->toString() + "*";
}

TypeKind Pointer::getKind() {
  return TypeKind::Pointer;
}

Pointer::~Pointer() = default;

Function::Function(TypeRef r, std::vector<TypeRef> p):
  returnType(r),
  params(p)
{}

std::string Function::toString() {
  std::stringstream ss;
  ss << "("
    << returnType->toString()
    << "(*)(";
  for (auto param : params) {
    ss << param->toString() << ", ";
  }
  ss <<  "))";
  return ss.str();
}

TypeKind Function::getKind() {
  return TypeKind::Function;
}

Function::~Function() = default;

Array::Array(TypeRef elem, size_t s):
  elementType(elem),
  size(std::make_shared<size_t>(s))
{}

std::string Array::toString() {
  return elementType->toString() + "[]";
}

TypeKind Array::getKind() {
  return TypeKind::Pointer;
}

Array::~Array() = default;

/*!
 * \brief Return the kind of \c type.
 */
TypeKind typeKind(TypeRef type) {
  return type->getKind();
}

TypeRef makeReservedType(std::string name) {
  return std::make_shared<Reserved>(
    Reserved(name)
  );
}

TypeRef makePointerType(TypeRef ref) {
  return std::make_shared<Pointer>(
    Pointer(ref)
  );
}

TypeRef makeFunctionType(
    TypeRef returnType,
    const std::vector<TypeRef>& params
) {
  return std::make_shared<Function>(
    Function(returnType, params)
  );
}

TypeRef makeArrayType(TypeRef elem, size_t size) {
  return std::make_shared<Array>(
    Array(elem, size)
  );
}

ReservedType getReservedType(TypeRef type) {
  assert(type->getKind() == TypeKind::Reserved);
  return {type->name};
}

PointerType getPointerType(TypeRef type) {
  assert(type->getKind() == TypeKind::Pointer);
  return {type->type};
}

FunctionType getFunctionType(TypeRef type) {
  assert(type->getKind() == TypeKind::Function);
  return {type->type, type->params};
}

ArrayType getArrayType(TypeRef type) {
  assert(type->getKind() == TypeKind::Array);
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
