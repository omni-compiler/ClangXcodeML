#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include "Symbol.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "TypeAnalyzer.h"
#include "llvm/Support/Casting.h"

#include <iostream>

std::string cv_qualify(
    const XcodeMl::TypeRef& type,
    const std::string& var
) {
  std::string str(var);
  if (type->isConst()) {
    str = type->addConstQualifier(str);
  }
  if (type->isVolatile()) {
    str = type->addVolatileQualifier(str);
  }
  return str;
}

namespace XcodeMl {

Type::Type(TypeKind k, std::string id, bool c, bool v):
  kind(k),
  ident(id),
  constness(c),
  volatility(v)
{}

Type::~Type() {}

std::string Type::addConstQualifier(std::string var) const {
  return static_cast<std::string>("const ") + var;
}

std::string Type::addVolatileQualifier(std::string var) const {
  return static_cast<std::string>("volatile ") + var;
}

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
  ss << var
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
  return makeDecl(returnType, ss.str(), env);
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

Array::Array(DataTypeIdent ident, DataTypeIdent elem, Array::Size s):
  Type(TypeKind::Array, ident),
  element(elem),
  size(s)
{}

Array::Array(DataTypeIdent ident, DataTypeIdent elem, size_t s):
  Type(TypeKind::Array, ident),
  element(elem),
  size(Size::makeIntegerSize(s))
{}

std::string Array::makeDeclaration(std::string var, const Environment& env) {
  auto elementType(env[element]);
  if (!elementType) {
    return "INCOMPLETE_TYPE *" + var;
  }
  const std::string size_expression =
    size.kind == Size::Kind::Integer ? std::to_string(size.size):"*";
  const std::string declarator =
    static_cast<std::string>("[") +
    static_cast<std::string>(isConst() ? "const ":"") +
    static_cast<std::string>(isVolatile() ? "volatile ":"") +
    size_expression +
    static_cast<std::string>("]");
  return makeDecl(elementType, var + declarator, env);
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

std::string Array::addConstQualifier(std::string var) const {
  // add cv-qualifiers in Array::makeDeclaration, not here
  return var;
}

std::string Array::addVolatileQualifier(std::string var) const {
  // add cv-qualifiers in Array::makeDeclaration, not here
  return var;
}

Array::Size::Size(Kind k, size_t s):
  kind(k),
  size(s)
{}

Array::Size Array::Size::makeIntegerSize(size_t s) {
  return Size(Kind::Integer, s);
}

Array::Size Array::Size::makeVariableSize() {
  return Size(Kind::Variable, 0);
}

Struct::Struct(
    const DataTypeIdent& ident,
    const std::string& t,
    const Struct::MemberList& f):
  Type(TypeKind::Struct, ident),
  tag(t),
  fields(f)
{}

std::string Struct::makeDeclaration(std::string var, const Environment&)
{
  std::stringstream ss;
  ss << "struct " << tag << " " << var;
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
  tag(other.tag),
  fields(other.fields)
{}

void Struct::setTagName(const std::string& tagname) {
  assert(tag == "");
  tag = tagname;
}

Struct::MemberList Struct::members() const {
  return fields;
}

std::string Struct::tagName() const {
  return tag;
}

Struct::BitSize::BitSize():
  valid(false),
  size_(0)
{}

Struct::BitSize::BitSize(size_t s):
  valid(true),
  size_(s)
{}

bool Struct::BitSize::isValid() const {
  return valid;
}

size_t Struct::BitSize::size() const {
  return size_;
}

Struct::Member::Member(
    const std::string& type,
    const std::string& name
):
  dataTypeIdent(type),
  name_(name),
  size()
{}

Struct::Member::Member(
    const std::string& type,
    const std::string& name,
    size_t s
):
  dataTypeIdent(type),
  name_(name),
  size(s)
{}

std::string Struct::Member::type() const {
  return dataTypeIdent;
}

std::string Struct::Member::name() const {
  return name_;
}

bool Struct::Member::isBitField() const {
  return size.isValid();
}

size_t Struct::Member::getSize() const {
  assert(isBitField());
  return size.size();
}

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
      elemType->dataTypeIdent(),
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

TypeRef makeArrayType(
    DataTypeIdent ident,
    TypeRef elemType,
    Array::Size size
) {
  return std::make_shared<Array>(
      ident,
      elemType->dataTypeIdent(),
      size
  );
}

TypeRef makeArrayType(
    DataTypeIdent ident,
    DataTypeIdent elemName,
    Array::Size size
) {
  return std::make_shared<Array>(ident, elemName, size);
}

TypeRef makeStructType(
    const DataTypeIdent& ident,
    const std::string& tag,
    const Struct::MemberList& fields
) {
  return std::make_shared<Struct>(ident, tag, fields);
}

std::string TypeRefToString(TypeRef type, const Environment& env) {
  return makeDecl(type, "", env);
}

}
