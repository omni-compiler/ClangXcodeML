#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "XcodeMlType.h"

namespace XcodeMl {

Type::~Type() {}

Reserved::Reserved(std::string dataType):
  name(dataType)
{}

std::string Reserved::makeDeclaration(std::string var) {
  return name + " " + var;
}

TypeKind Reserved::getKind() {
  return TypeKind::Reserved;
}

Reserved::~Reserved() = default;

Pointer::Pointer(TypeRef signified):
  ref(signified)
{}

std::string Pointer::makeDeclaration(std::string var) {
  assert(ref);
  return ref->makeDeclaration("*" + var);
}

TypeKind Pointer::getKind() {
  return TypeKind::Pointer;
}

Pointer::~Pointer() = default;

Function::Function(TypeRef r, const std::vector<TypeRef>& p):
  returnType(r),
  params(p.size())
{
  // FIXME: initialization cost
  for (size_t i = 0; i < p.size(); ++i) {
    params[i] = std::make_tuple(p[i], "");
  }
}

Function::Function(TypeRef r, const std::vector<std::tuple<TypeRef, std::string>>& p):
  returnType(r),
  params(p)
{}

std::string Function::makeDeclaration(std::string var) {
  std::stringstream ss;
  ss << returnType->makeDeclaration("")
    << " "
    << var
    << "(";
  for (auto param : params) {
    auto paramType(std::get<0>(param));
    auto paramName(std::get<1>(param));
    ss << paramType->makeDeclaration(paramName) << ", ";
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

std::string Array::makeDeclaration(std::string var) {
  return elementType->makeDeclaration(var + "[]");
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

std::string TypeRefToString(TypeRef type) {
  return type->makeDeclaration("");
}

}
