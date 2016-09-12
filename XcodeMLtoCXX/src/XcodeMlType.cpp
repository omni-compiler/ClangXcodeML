#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include "SymbolAnalyzer.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "TypeAnalyzer.h"
#include "llvm/Support/Casting.h"

#include <iostream>

std::string cv_qualify(const XcodeMl::TypeRef& type, const std::string var) {
  return
      static_cast<std::string>(type->isConst() ? "const ":"") +
      static_cast<std::string>(type->isVolatile() ? "volatile ":"") +
      var;
}

namespace XcodeMl {

Type::Type(TypeKind k, std::string id, bool c, bool v):
  kind(k),
  ident(id),
  constness(c),
  volatility(v)
{}

Type::~Type() {}

bool Type::isConst() const {
  return constness;
}

bool Type::isVolatile() const {
  return volatility;
}

void Type::setConst(bool c) {
  constness = c;
}

void Type::setVolatile(bool v) {
  volatility = v;
}

DataTypeIdent Type::dataTypeIdent() {
  return ident;
}

TypeKind Type::getKind() const {
  return kind;
}

Type::Type(const Type& other):
  kind(other.kind),
  ident(other.ident),
  constness(other.constness),
  volatility(other.volatility)
{}

Reserved::Reserved(DataTypeIdent ident, std::string dataType):
  Type(TypeKind::Reserved, ident),
  name(dataType)
{}

std::string Reserved::makeDeclaration(std::string var, const Environment&) {
  return name + " " + var;
}

Reserved::~Reserved() = default;

Type* Reserved::clone() const {
  Reserved* copy = new Reserved(*this);
  return copy;
}

Reserved::Reserved(const Reserved& other):
  Type(other),
  name(other.name)
{}

bool Reserved::classof(const Type* T) {
  return T->getKind() == TypeKind::Reserved;
}

Pointer::Pointer(DataTypeIdent ident, TypeRef signified):
  Type(TypeKind::Pointer, ident),
  ref(signified->dataTypeIdent())
{}

Pointer::Pointer(DataTypeIdent ident, DataTypeIdent signified):
  Type(TypeKind::Pointer, ident),
  ref(signified)
{}

std::string Pointer::makeDeclaration(std::string var, const Environment& env) {
  auto refType = env[ref];
  if (!refType) {
    return "INCOMPLETE_TYPE *" + var;
  }
  switch (typeKind(refType)) {
    case TypeKind::Function:
      return makeDecl(refType, "(*" + var + ")", env);
    default:
      return makeDecl(refType, "* " + var, env);
  }
}

Pointer::~Pointer() = default;

Type* Pointer::clone() const {
  Pointer* copy = new Pointer(*this);
  return copy;
}

bool Pointer::classof(const Type* T) {
  return T->getKind() == TypeKind::Pointer;
}

Pointer::Pointer(const Pointer& other):
  Type(other),
  ref(other.ref)
{}

Function::Function(DataTypeIdent ident, TypeRef r, const std::vector<DataTypeIdent>& p):
  Type(TypeKind::Function, ident),
  returnValue(r->dataTypeIdent()),
  params(p.size())
{
  // FIXME: initialization cost
  for (size_t i = 0; i < p.size(); ++i) {
    params[i] = std::make_tuple(p[i], "");
  }
}

Function::Function(DataTypeIdent ident, TypeRef r, const std::vector<std::tuple<DataTypeIdent, std::string>>& p):
  Type(TypeKind::Function, ident),
  returnValue(r->dataTypeIdent()),
  params(p)
{}

std::string Function::makeDeclaration(std::string var, const Environment& env) {
  std::stringstream ss;
  auto returnType(env[returnValue]);
  if (!returnType) {
    return "INCOMPLETE_TYPE *" + var;
  }
  ss << makeDecl(returnType, "", env)
    << " "
    << var
    << "(";
  bool alreadyPrinted = false;
  for (auto param : params) {
    auto paramDTI(std::get<0>(param));
    auto paramType(env[paramDTI]);
    auto paramName(std::get<1>(param));
    if (!paramType) {
      return "INCOMPLETE_TYPE *" + var;
    }
    ss << (alreadyPrinted ? ", " : "")
      << makeDecl(paramType, paramName, env);
    alreadyPrinted = true;
  }
  ss <<  ")";
  return ss.str();
}

Function::~Function() = default;

Type* Function::clone() const {
  Function* copy = new Function(*this);
  return copy;
}

bool Function::classof(const Type* T) {
  return T->getKind() == TypeKind::Function;
}

Function::Function(const Function& other):
  Type(other),
  returnValue(other.returnValue),
  params(other.params)
{}

Array::Array(DataTypeIdent ident, TypeRef elem, size_t s):
  Type(TypeKind::Array, ident),
  element(elem->dataTypeIdent()),
  size(std::make_shared<size_t>(s))
{}

Array::Array(DataTypeIdent ident, DataTypeIdent elem, size_t s):
  Type(TypeKind::Array, ident),
  element(elem),
  size(std::make_shared<size_t>(s))
{}

std::string Array::makeDeclaration(std::string var, const Environment& env) {
  auto elementType(env[element]);
  if (!elementType) {
    return "INCOMPLETE_TYPE *" + var;
  }
  return makeDecl(elementType, var + "[]", env);
}

Array::~Array() = default;

Type* Array::clone() const {
  Array* copy = new Array(*this);
  return copy;
}

bool Array::classof(const Type* T) {
  return T->getKind() == TypeKind::Array;
}

Array::Array(const Array& other):
  Type(other),
  element(other.element),
  size(other.size)
{}

Struct::Struct(DataTypeIdent ident, std::string n, std::string t, SymbolMap &&f)
  : Type(TypeKind::Struct, ident), name(n), tag(t), fields(f) {
  std::cerr << "Struct::Struct(" << n << ")" << std::endl;
}

std::string Struct::makeDeclaration(std::string var, const Environment&)
{
  std::stringstream ss;
  ss << "struct " << name << " " << var;
  return ss.str();
}

Struct::~Struct() = default;

Type* Struct::clone() const {
  Struct* copy = new Struct(*this);
  return copy;
}

bool Struct::classof(const Type* T) {
  return T->getKind() == TypeKind::Struct;
}

Struct::Struct(const Struct& other):
  Type(other),
  name(other.name),
  tag(other.tag),
  fields(other.fields)
{}

/*!
 * \brief Return the kind of \c type.
 */
TypeKind typeKind(TypeRef type) {
  return type->getKind();
}

std::string makeDecl(TypeRef type, std::string var, const Environment& env) {
  if (type) {
    return type->makeDeclaration(cv_qualify(type, var), env);
  } else {
    return "UNKNOWN_TYPE";
  }
}

TypeRef makeReservedType(DataTypeIdent ident, std::string name, bool c, bool v) {
  auto type = std::make_shared<Reserved>(
      ident,
      name
  );
  type->setConst(c);
  type->setVolatile(v);
  return type;
}

TypeRef makePointerType(DataTypeIdent ident, TypeRef ref) {
  return std::make_shared<Pointer>(
      ident,
      ref
  );
}

TypeRef makePointerType(DataTypeIdent ident, DataTypeIdent ref) {
  return std::make_shared<Pointer>(ident, ref);
}

TypeRef makeFunctionType(
    DataTypeIdent ident,
    TypeRef returnType,
    const Function::Params& params
) {
  return std::make_shared<Function>(
      ident,
      returnType,
      params
  );
}

TypeRef makeArrayType(
    DataTypeIdent ident,
    TypeRef elemType,
    size_t size
) {
  return std::make_shared<Array>(
      ident,
      elemType,
      size
  );
}

TypeRef makeArrayType(
    DataTypeIdent ident,
    DataTypeIdent elemType,
    size_t size
) {
  return std::make_shared<Array>(ident, elemType, size);
}

TypeRef makeStructType(
    DataTypeIdent ident,
    std::string name,
    std::string tag,
    SymbolMap&& fields
) {
  return std::make_shared<Struct>(ident, name, tag, std::move(fields));
}

std::string TypeRefToString(TypeRef type, const Environment& env) {
  return makeDecl(type, "", env);
}

}
