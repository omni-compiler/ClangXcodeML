#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include "StringTree.h"
#include "Symbol.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "llvm/Support/Casting.h"

#include <iostream>

using XcodeMl::CodeFragment;
using CXXCodeGen::makeTokenNode;
using CXXCodeGen::makeVoidNode;

CodeFragment cv_qualify(
    const XcodeMl::TypeRef& type,
    const CodeFragment& var
) {
  CodeFragment str(var);
  if (type->isConst()) {
    str = type->addConstQualifier(str);
  }
  if (type->isVolatile()) {
    str = type->addVolatileQualifier(str);
  }
  return str;
}

namespace XcodeMl {

Type::Type(TypeKind k, DataTypeIdent id, bool c, bool v):
  kind(k),
  ident(id),
  constness(c),
  volatility(v)
{}

Type::~Type() {}

CodeFragment Type::addConstQualifier(CodeFragment var) const {
  return makeTokenNode("const") + var;
}

CodeFragment Type::addVolatileQualifier(CodeFragment var) const {
  return makeTokenNode("volatile") + var;
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

Reserved::Reserved(DataTypeIdent ident, CodeFragment dataType):
  Type(TypeKind::Reserved, ident),
  name(dataType)
{}

CodeFragment Reserved::makeDeclaration(CodeFragment var, const Environment&) {
  return name + var;
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

CodeFragment Pointer::makeDeclaration(CodeFragment var, const Environment& env) {
  auto refType = env[ref];
  if (!refType) {
    return makeTokenNode( "INCOMPLETE_TYPE *" ) + var;
  }
  switch (typeKind(refType)) {
    case TypeKind::Function:
      return makeDecl(
          refType,
          makeTokenNode("(*") + var + makeTokenNode( ")" ),
          env);
    default:
      return makeDecl(refType, makeTokenNode("*") + var, env);
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
    params[i] = std::make_tuple(p[i], makeVoidNode());
  }
}

Function::Function(DataTypeIdent ident, TypeRef r, const std::vector<std::tuple<DataTypeIdent, CodeFragment>>& p):
  Type(TypeKind::Function, ident),
  returnValue(r->dataTypeIdent()),
  params(p)
{}

CodeFragment Function::makeDeclaration(
    CodeFragment var,
    const std::vector<CodeFragment>& args,
    const Environment& env)
{
  assert(isParamListEmpty() || args.size() == params.size());
  if (!env.exists(returnValue)) {
    return makeTokenNode("INCOMPLETE_TYPE *") + var;
  }
  auto returnType = env[returnValue];
  auto decl = var + makeTokenNode("(");
  bool alreadyPrinted = false;
  for (int i = 0, len = args.size(); i < len; ++i) {
    if (!env.exists(std::get<0>(params[i]))) {
      return makeTokenNode( "INCOMPLETE_TYPE *" ) + var;
    }
    if (alreadyPrinted) {
      decl = decl + makeTokenNode(",");
    }
    auto paramType = env[std::get<0>(params[i])];
    decl = decl + makeDecl(paramType, args[i], env);
    alreadyPrinted = true;
  }
  decl = decl + makeTokenNode(")");
  return
    returnType ?
        makeDecl(returnType, decl, env)
      : decl;
}

CodeFragment Function::makeDeclaration(CodeFragment var, const Environment& env) {
  std::vector<CodeFragment> vec;
  for (auto param : params) {
    auto paramName(std::get<1>(param));
    vec.push_back(paramName);
  }
  return makeDeclaration(var, vec, env);
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

bool Function::isParamListEmpty() const {
  if (params.empty()) {
    return true;
  }
  return
    params.size() == 1 &&
    std::get<0>(params.at(0)) == "void";
}

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

CodeFragment Array::makeDeclaration(CodeFragment var, const Environment& env) {
  auto elementType(env[element]);
  if (!elementType) {
    return makeTokenNode("INCOMPLETE_TYPE *") + var;
  }
  const CodeFragment size_expression =
    size.kind == Size::Kind::Integer ?
        makeTokenNode(std::to_string(size.size))
      : makeTokenNode("*");
  const CodeFragment declarator =
    makeTokenNode("[") +
    (isConst() ? makeTokenNode("const") : makeVoidNode()) +
    (isConst() ? makeTokenNode("volatile") : makeVoidNode()) +
    size_expression +
    makeTokenNode("]");
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

CodeFragment Array::addConstQualifier(CodeFragment var) const {
  // add cv-qualifiers in Array::makeDeclaration, not here
  return var;
}

CodeFragment Array::addVolatileQualifier(CodeFragment var) const {
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
    const CodeFragment& t,
    const Struct::MemberList& f):
  Type(TypeKind::Struct, ident),
  tag(t),
  fields(f)
{}

CodeFragment Struct::makeDeclaration(CodeFragment var, const Environment&)
{
  return makeTokenNode("struct") + tag + var;
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

void Struct::setTagName(const CodeFragment& tagname) {
  tag = tagname;
}

Struct::MemberList Struct::members() const {
  return fields;
}

CodeFragment Struct::tagName() const {
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
    const DataTypeIdent& type,
    const CodeFragment& name
):
  dataTypeIdent(type),
  name_(name),
  size()
{}

Struct::Member::Member(
    const DataTypeIdent& type,
    const CodeFragment& name,
    size_t s
):
  dataTypeIdent(type),
  name_(name),
  size(s)
{}

DataTypeIdent Struct::Member::type() const {
  return dataTypeIdent;
}

CodeFragment Struct::Member::name() const {
  return name_;
}

bool Struct::Member::isBitField() const {
  return size.isValid();
}

size_t Struct::Member::getSize() const {
  assert(isBitField());
  return size.size();
}

std::string string_of_accessSpec(AccessSpec as) {
  switch (as) {
    case AccessSpec::Public:
      return "public";
    case AccessSpec::Private:
      return "private";
    case AccessSpec::Protected:
      return "private";
    default:
      assert(false);
  }
}

AccessSpec accessSpec_of_string(const std::string& as) {
  if (as == "public") {
    return AccessSpec::Public;
  } else if (as == "private") {
    return AccessSpec::Private;
  } else if (as == "protected") {
    return AccessSpec::Protected;
  } else {
    const auto what =
      static_cast<std::string>(
          "Expected "
          "\"public\", \"private\", ""\"protected\", "
          "but got ")
      + as;
    throw std::invalid_argument(what);
  }
}

ClassType::ClassType(
    const DataTypeIdent& ident,
    const CodeFragment& className,
    xmlNodePtr node):
  Type(TypeKind::Class, ident),
  name_(className),
  declNode(node)
{}

CodeFragment ClassType::makeDeclaration(
    CodeFragment var,
    const Environment&
) {
  return name_ + var;
}

Type* ClassType::clone() const {
  ClassType* copy = new ClassType(*this);
  return copy;
}

xmlNodePtr ClassType::getNode() const {
  return declNode;
}

CodeFragment ClassType::name() const {
  return name_;
}

bool ClassType::classof(const Type *T) {
  return T->getKind() == TypeKind::Class;
}

ClassType::ClassType(const ClassType& other):
  Type(other),
  name_(other.name_),
  declNode(other.declNode)
{}

/*!
 * \brief Return the kind of \c type.
 */
TypeKind typeKind(TypeRef type) {
  return type->getKind();
}

CodeFragment makeDecl(TypeRef type, CodeFragment var, const Environment& env) {
  if (type) {
    return type->makeDeclaration(cv_qualify(type, var), env);
  } else {
    return makeTokenNode( "UNKNOWN_TYPE" );
  }
}

TypeRef makeReservedType(DataTypeIdent ident, CodeFragment name, bool c, bool v) {
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
    const CodeFragment& tag,
    const Struct::MemberList& fields
) {
  return std::make_shared<Struct>(ident, tag, fields);
}

CodeFragment TypeRefToString(TypeRef type, const Environment& env) {
  return makeDecl(type, makeTokenNode( "" ), env);
}

}
