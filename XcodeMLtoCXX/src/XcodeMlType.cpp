#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "SymbolAnalyzer.h"
#include "XcodeMlType.h"

#include <iostream>

namespace XcodeMl {

Type::Type(std::string id):
  ident(id)
{}

Type::~Type() {}

DataTypeIdent Type::dataTypeIdent() {
  return ident;
}

Reserved::Reserved(DataTypeIdent ident, std::string dataType):
  Type(ident),
  name(dataType)
{}

std::string Reserved::makeDeclaration(std::string var) {
  return name + " " + var;
}

TypeKind Reserved::getKind() {
  return TypeKind::Reserved;
}

Reserved::~Reserved() = default;

Pointer::Pointer(DataTypeIdent ident, TypeRef signified):
  Type(ident),
  ref(signified->dataTypeIdent())
{}

std::string Pointer::makeDeclaration(std::string var) {
  if (!ref) {
    return "INCOMPLETE_TYPE *" + var;
  }
  switch (typeKind(ref)) {
    case TypeKind::Function:
      return makeDecl(ref, "(*" + var + ")");
    default:
      return makeDecl(ref, "*" + var);
  }
}

TypeKind Pointer::getKind() {
  return TypeKind::Pointer;
}

Pointer::~Pointer() = default;

Function::Function(DataTypeIdent ident, TypeRef r, const std::vector<TypeRef>& p):
  Type(ident),
  returnType(r->dataTypeIdent()),
  params(p.size())
{
  // FIXME: initialization cost
  for (size_t i = 0; i < p.size(); ++i) {
    params[i] = std::make_tuple(p[i]->dataTypeIdent(), "");
  }
}

Function::Function(DataTypeIdent ident, TypeRef r, const std::vector<std::tuple<TypeRef, std::string>>& p):
  Type(ident),
  returnType(r->dataTypeIdent()),
  params(p)
{}

std::string Function::makeDeclaration(std::string var) {
  std::stringstream ss;
  ss << makeDecl(returnType, "")
    << " "
    << var
    << "(";
  for (auto param : params) {
    auto paramType(std::get<0>(param));
    auto paramName(std::get<1>(param));
    ss << makeDecl(paramType, paramName) << ", ";
  }
  ss <<  ")";
  return ss.str();
}

TypeKind Function::getKind() {
  return TypeKind::Function;
}

Function::~Function() = default;

Array::Array(TypeRef elem, size_t s):
  elementType(elem->dataTypeIdent()),
  size(std::make_shared<size_t>(s))
{}

std::string Array::makeDeclaration(std::string var) {
  return makeDecl(elementType, var + "[]");
}

TypeKind Array::getKind() {
  return TypeKind::Pointer;
}

Array::~Array() = default;

Struct::Struct(std::string n, std::string t, SymbolMap &&f)
  : name(n), tag(t), fields(f) {
  std::cerr << "Struct::Struct(" << n << ")" << std::endl;
}

std::string Struct::makeDeclaration(std::string var)
{
  std::stringstream ss;
  ss << "struct " << name << " " << var;
  return ss.str();
}

Struct::~Struct() = default;

TypeKind Struct::getKind() {
  return TypeKind::Struct;
}

/*!
 * \brief Return the kind of \c type.
 */
TypeKind typeKind(TypeRef type) {
  return type->getKind();
}

std::string makeDecl(TypeRef type, std::string var) {
  if (type) {
    return type->makeDeclaration(var);
  } else {
    return "UNKNOWN_TYPE";
  }
}

std::string TypeRefToString(TypeRef type) {
  return makeDecl(type, "");
}

}
