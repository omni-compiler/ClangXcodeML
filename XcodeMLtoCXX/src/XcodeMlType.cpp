#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include "llvm/ADT/Optional.h"
#include "StringTree.h"
#include "XcodeMlType.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"
#include "XcodeMlEnvironment.h"
#include "llvm/Support/Casting.h"

#include <iostream>

using XcodeMl::CodeFragment;
using CXXCodeGen::makeTokenNode;
using CXXCodeGen::makeVoidNode;

CodeFragment
cv_qualify(const XcodeMl::TypeRef &type, const CodeFragment &var) {
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

MemberDecl::MemberDecl(const DataTypeIdent &d, const CodeFragment &c)
    : dtident(d), name(c), bitfield() {
}

MemberDecl::MemberDecl(const DataTypeIdent &d, const CodeFragment &c, size_t s)
    : dtident(d), name(c), bitfield(s) {
}

CodeFragment
MemberDecl::makeDeclaration(const Environment &env) const {
  auto type = env.at(dtident);
  return makeDecl(type, name, env)
      + (bitfield ? makeTokenNode(std::to_string(*bitfield)) : makeVoidNode());
}

Type::Type(TypeKind k, DataTypeIdent id, bool c, bool v)
    : kind(k), ident(id), constness(c), volatility(v) {
}

Type::~Type() {
}

CodeFragment
Type::addConstQualifier(CodeFragment var) const {
  return makeTokenNode("const") + var;
}

CodeFragment
Type::addVolatileQualifier(CodeFragment var) const {
  return makeTokenNode("volatile") + var;
}

bool
Type::isConst() const {
  return constness;
}

bool
Type::isVolatile() const {
  return volatility;
}

void
Type::setConst(bool c) {
  constness = c;
}

void
Type::setVolatile(bool v) {
  volatility = v;
}

DataTypeIdent
Type::dataTypeIdent() {
  return ident;
}

TypeKind
Type::getKind() const {
  return kind;
}

Type::Type(const Type &other)
    : kind(other.kind),
      ident(other.ident),
      constness(other.constness),
      volatility(other.volatility) {
}

Reserved::Reserved(DataTypeIdent ident, CodeFragment dataType)
    : Type(TypeKind::Reserved, ident), name(dataType) {
}

CodeFragment
Reserved::makeDeclaration(CodeFragment var, const Environment &) {
  return name + var;
}

Reserved::~Reserved() = default;

Type *
Reserved::clone() const {
  Reserved *copy = new Reserved(*this);
  return copy;
}

Reserved::Reserved(const Reserved &other) : Type(other), name(other.name) {
}

bool
Reserved::classof(const Type *T) {
  return T->getKind() == TypeKind::Reserved;
}

QualifiedType::QualifiedType(DataTypeIdent ident,
    DataTypeIdent baseType,
    bool constness,
    bool volatility)
    : Type(TypeKind::Qualified, ident),
      underlying(baseType),
      isConst(constness),
      isVolatile(volatility) {
}

CodeFragment
QualifiedType::makeDeclaration(CodeFragment var, const Environment &env) {
  const auto T = env[underlying];
  TypeRef QT(T->clone());
  if (isConst) {
    QT->setConst(true);
  }
  if (isVolatile) {
    QT->setVolatile(true);
  }
  return makeDecl(QT, var, env);
}

QualifiedType::~QualifiedType() = default;

Type *
QualifiedType::clone() const {
  QualifiedType *copy = new QualifiedType(*this);
  return copy;
}

bool
QualifiedType::classof(const Type *T) {
  return T->getKind() == TypeKind::Qualified;
}

QualifiedType::QualifiedType(const QualifiedType &other)
    : Type(other),
      underlying(other.underlying),
      isConst(other.isConst),
      isVolatile(other.isVolatile) {
}

Pointer::Pointer(DataTypeIdent ident, TypeRef signified)
    : Type(TypeKind::Pointer, ident), ref(signified->dataTypeIdent()) {
}

Pointer::Pointer(DataTypeIdent ident, DataTypeIdent signified)
    : Type(TypeKind::Pointer, ident), ref(signified) {
}

CodeFragment
Pointer::makeDeclaration(CodeFragment var, const Environment &env) {
  auto refType = env[ref];
  if (!refType) {
    return makeTokenNode("INCOMPLETE_TYPE *") + var;
  }
  switch (typeKind(refType)) {
  case TypeKind::Function:
    return makeDecl(
        refType, makeTokenNode("(*") + var + makeTokenNode(")"), env);
  default: return makeDecl(refType, makeTokenNode("*") + var, env);
  }
}

Pointer::~Pointer() = default;

Type *
Pointer::clone() const {
  Pointer *copy = new Pointer(*this);
  return copy;
}

bool
Pointer::classof(const Type *T) {
  return T->getKind() == TypeKind::Pointer;
}

TypeRef
Pointer::getPointee(const Environment &env) const {
  return env.at(ref);
}

Pointer::Pointer(const Pointer &other) : Type(other), ref(other.ref) {
}

ReferenceType::ReferenceType(
    const DataTypeIdent &ident, TypeKind kind, const DataTypeIdent &r)
    : Type(kind, ident), ref(r) {
}

ReferenceType::~ReferenceType() = default;

TypeRef
ReferenceType::getPointee(const Environment &env) const {
  return env.at(ref);
}

LValueReferenceType::LValueReferenceType(
    const DataTypeIdent &ident, const DataTypeIdent &ref)
    : ReferenceType(ident, TypeKind::LValueReference, ref) {
}

CodeFragment
LValueReferenceType::makeDeclaration(
    CodeFragment var, const Environment &env) {
  return makeDecl(env.at(ref), makeTokenNode("&") + var, env);
}

Type *
LValueReferenceType::clone() const {
  LValueReferenceType *copy = new LValueReferenceType(*this);
  return copy;
}

bool
LValueReferenceType::classof(const Type *T) {
  return T->getKind() == TypeKind::LValueReference;
}

ParamList::ParamList(const std::vector<DataTypeIdent> &d, bool e)
    : dtidents(d), hasEllipsis(e) {
}

bool
ParamList::isVariadic() const {
  return hasEllipsis;
}

bool
ParamList::isEmpty() const {
  if (dtidents.empty()) {
    return true;
  }
  return dtidents.size() == 1 && dtidents[0] == "void";
}

CodeFragment
ParamList::makeDeclaration(
    const std::vector<CodeFragment> &vars, const Environment &env) const {
  assert(dtidents.size() == vars.size());
  std::vector<CodeFragment> decls;
  for (int i = 0, len = dtidents.size(); i < len; ++i) {
    decls.push_back(makeDecl(env[dtidents[i]], vars[i], env));
  }
  return CXXCodeGen::join(",", decls)
      + (isVariadic() ? makeTokenNode(",") + makeTokenNode("...")
                      : makeVoidNode());
}

Function::Function(DataTypeIdent ident,
    TypeRef r,
    const std::vector<DataTypeIdent> &p,
    bool v)
    : Type(TypeKind::Function, ident),
      returnValue(r->dataTypeIdent()),
      params(p, v),
      defaultArgs(p.size(), makeVoidNode()) {
}

Function::Function(DataTypeIdent ident, TypeRef r, const Params &p, bool v)
    : Type(TypeKind::Function, ident),
      returnValue(r->dataTypeIdent()),
      params(),
      defaultArgs() {
  std::vector<DataTypeIdent> dtidents;
  for (auto param : p) {
    dtidents.push_back(std::get<0>(param));
    defaultArgs.push_back(std::get<1>(param));
  }
  params = ParamList(dtidents, v);
}

CodeFragment
Function::makeDeclarationWithoutReturnType(CodeFragment var,
    const std::vector<CodeFragment> &args,
    const Environment &env) {
  auto decl = var + makeTokenNode("(");
  decl = decl + params.makeDeclaration(args, env);
  decl = decl + makeTokenNode(")");
  if (isConst()) {
    decl = decl + makeTokenNode("const");
  }
  if (isVolatile()) {
    decl = decl + makeTokenNode("volatile");
  }
  return decl;
}

CodeFragment
Function::makeDeclarationWithoutReturnType(
    CodeFragment var, const Environment &env) {
  return makeDeclarationWithoutReturnType(var, defaultArgs, env);
}

CodeFragment
Function::makeDeclaration(CodeFragment var,
    const std::vector<CodeFragment> &args,
    const Environment &env) {
  auto returnType = env.at(returnValue);
  auto decl = makeDeclarationWithoutReturnType(var, args, env);
  return makeDecl(returnType, decl, env);
}

CodeFragment
Function::makeDeclaration(CodeFragment var, const Environment &env) {
  return makeDeclaration(var, defaultArgs, env);
}

std::vector<CodeFragment>
Function::argNames() const {
  return defaultArgs;
}

CodeFragment
Function::addConstQualifier(CodeFragment var) const {
  // add cv-qualifiers in Function::makeDeclaration, not here
  return var;
}

CodeFragment
Function::addVolatileQualifier(CodeFragment var) const {
  // add cv-qualifiers in Function::makeDeclaration, not here
  return var;
}

Function::~Function() = default;

Type *
Function::clone() const {
  Function *copy = new Function(*this);
  return copy;
}

bool
Function::classof(const Type *T) {
  return T->getKind() == TypeKind::Function;
}

Function::Function(const Function &other)
    : Type(other), returnValue(other.returnValue), params(other.params) {
}

bool
Function::isParamListEmpty() const {
  return params.isEmpty();
}

Array::Array(DataTypeIdent ident, DataTypeIdent elem, Array::Size s)
    : Type(TypeKind::Array, ident), element(elem), size(s) {
}

Array::Array(DataTypeIdent ident, DataTypeIdent elem, size_t s)
    : Type(TypeKind::Array, ident),
      element(elem),
      size(Size::makeIntegerSize(s)) {
}

CodeFragment
Array::makeDeclaration(CodeFragment var, const Environment &env) {
  auto elementType(env[element]);
  if (!elementType) {
    return makeTokenNode("INCOMPLETE_TYPE *") + var;
  }
  const CodeFragment size_expression = size.kind == Size::Kind::Integer
      ? makeTokenNode(std::to_string(size.size))
      : makeTokenNode("*");
  const CodeFragment declarator = makeTokenNode("[")
      + (isConst() ? makeTokenNode("const") : makeVoidNode())
      + (isConst() ? makeTokenNode("volatile") : makeVoidNode())
      + size_expression + makeTokenNode("]");
  return makeDecl(elementType, var + declarator, env);
}

Array::~Array() = default;

Type *
Array::clone() const {
  Array *copy = new Array(*this);
  return copy;
}

bool
Array::classof(const Type *T) {
  return T->getKind() == TypeKind::Array;
}

Array::Array(const Array &other)
    : Type(other), element(other.element), size(other.size) {
}

CodeFragment
Array::addConstQualifier(CodeFragment var) const {
  // add cv-qualifiers in Array::makeDeclaration, not here
  return var;
}

CodeFragment
Array::addVolatileQualifier(CodeFragment var) const {
  // add cv-qualifiers in Array::makeDeclaration, not here
  return var;
}

TypeRef
Array::getElemType(const Environment &env) const {
  return env.at(element);
}

Array::Size::Size(Kind k, size_t s) : kind(k), size(s) {
}

Array::Size
Array::Size::makeIntegerSize(size_t s) {
  return Size(Kind::Integer, s);
}

Array::Size
Array::Size::makeVariableSize() {
  return Size(Kind::Variable, 0);
}

Struct::Struct(const DataTypeIdent &ident,
    const CodeFragment &t,
    const Struct::MemberList &f)
    : Type(TypeKind::Struct, ident), tag(t), fields(f) {
}

CodeFragment
Struct::makeDeclaration(CodeFragment var, const Environment &) {
  return makeTokenNode("struct") + tag + var;
}

CodeFragment
Struct::makeStructDefinition(const Environment &env) const {
  auto body = makeVoidNode();
  for (auto &field : fields) {
    body = body + field.makeDeclaration(env);
  }
  return makeTokenNode("struct") + makeTokenNode("{") + body
      + makeTokenNode("}") + makeTokenNode(";");
}

Struct::~Struct() = default;

Type *
Struct::clone() const {
  Struct *copy = new Struct(*this);
  return copy;
}

bool
Struct::classof(const Type *T) {
  return T->getKind() == TypeKind::Struct;
}

Struct::Struct(const Struct &other)
    : Type(other), tag(other.tag), fields(other.fields) {
}

void
Struct::setTagName(const CodeFragment &tagname) {
  tag = tagname;
}

Struct::MemberList
Struct::members() const {
  return fields;
}

CodeFragment
Struct::tagName() const {
  return tag;
}

EnumType::EnumType(const DataTypeIdent &ident, const EnumType::EnumName &name)
    : Type(TypeKind::Enum, ident), name_(name), declBody(makeVoidNode()) {
}

EnumType::EnumType(const DataTypeIdent &ident,
    const EnumType::EnumName &name,
    const CodeFragment &d)
    : Type(TypeKind::Enum, ident), name_(name), declBody(d) {
}

CodeFragment
EnumType::makeDeclaration(CodeFragment var, const Environment &) {
  return makeTokenNode("enum") + (name_ ? (*name_) : makeVoidNode()) + declBody
      + var;
}

Type *
EnumType::clone() const {
  EnumType *copy = new EnumType(*this);
  return copy;
}

bool
EnumType::classof(const Type *T) {
  return T->getKind() == TypeKind::Enum;
}

void
EnumType::setName(const std::string &enum_name) {
  assert(!name_);
  name_ = makeTokenNode(enum_name);
}

EnumType::EnumType(const EnumType &other)
    : Type(other), name_(other.name_), declBody(other.declBody) {
}

UnionType::UnionType(
    const DataTypeIdent &ident, const UnionType::UnionName &name)
    : Type(TypeKind::Union, ident), name_(name), members() {
}

UnionType::UnionType(const DataTypeIdent &ident,
    const UnionType::UnionName &name,
    const std::vector<MemberDecl> &v)
    : Type(TypeKind::Union, ident), name_(name), members(v) {
}

CodeFragment
UnionType::makeDeclaration(CodeFragment var, const Environment &env) {
  auto memberDecls = makeVoidNode();
  for (auto &member : members) {
    memberDecls = memberDecls + member.makeDeclaration(env);
  }
  return makeTokenNode("union") + (name_ ? (*name_) : makeVoidNode())
      + memberDecls + var;
}

Type *
UnionType::clone() const {
  UnionType *copy = new UnionType(*this);
  return copy;
}

bool
UnionType::classof(const Type *T) {
  return T->getKind() == TypeKind::Union;
}

void
UnionType::setName(const std::string &enum_name) {
  assert(!name_);
  name_ = makeTokenNode(enum_name);
}

UnionType::UnionType(const UnionType &other)
    : Type(other), name_(other.name_), members(other.members) {
}

std::string
string_of_accessSpec(AccessSpec as) {
  switch (as) {
  case AccessSpec::Public: return "public";
  case AccessSpec::Private: return "private";
  case AccessSpec::Protected: return "private";
  default: assert(false);
  }
}

AccessSpec
accessSpec_of_string(const std::string &as) {
  if (as == "public") {
    return AccessSpec::Public;
  } else if (as == "private") {
    return AccessSpec::Private;
  } else if (as == "protected") {
    return AccessSpec::Protected;
  } else {
    const auto what = static_cast<std::string>("Expected "
                                               "\"public\", \"private\", "
                                               "\"protected\", "
                                               "but got ")
        + as;
    throw std::invalid_argument(what);
  }
}

ClassType::ClassType(const DataTypeIdent &ident,
    const CodeFragment &className,
    const ClassType::Symbols &symbols)
    : Type(TypeKind::Class, ident),
      name_(className),
      bases_(),
      classScopeSymbols(symbols) {
}

ClassType::ClassType(const DataTypeIdent &ident,
    const CodeFragment &className,
    const std::vector<BaseClass> &b,
    const ClassType::Symbols &symbols)
    : Type(TypeKind::Class, ident),
      name_(className),
      bases_(b),
      classScopeSymbols(symbols) {
}

ClassType::ClassType(
    const DataTypeIdent &ident, const ClassType::Symbols &symbols)
    : Type(TypeKind::Class, ident),
      name_(),
      bases_(),
      classScopeSymbols(symbols) {
}

ClassType::ClassType(const DataTypeIdent &ident,
    const std::vector<BaseClass> &b,
    const ClassType::Symbols &symbols)
    : Type(TypeKind::Class, ident),
      name_(),
      bases_(b),
      classScopeSymbols(symbols) {
}

CodeFragment
ClassType::makeDeclaration(CodeFragment var, const Environment &) {
  assert(name_);
  return makeTokenNode("class") + *name_ + var;
}

Type *
ClassType::clone() const {
  ClassType *copy = new ClassType(*this);
  return copy;
}

ClassType::ClassName
ClassType::name() const {
  return name_;
}

void
ClassType::setName(const std::string &name) {
  name_ = makeTokenNode(name);
}

void
ClassType::setName(const CodeFragment &name) {
  name_ = name;
}
ClassType::Symbols
ClassType::getSymbols() const {
  return classScopeSymbols;
}

std::vector<ClassType::BaseClass>
ClassType::getBases() const {
  return bases_;
}

bool
ClassType::classof(const Type *T) {
  return T->getKind() == TypeKind::Class;
}

ClassType::ClassType(const ClassType &other)
    : Type(other),
      name_(other.name_),
      classScopeSymbols(other.classScopeSymbols) {
}

OtherType::OtherType(const DataTypeIdent &ident)
    : Type(TypeKind::Other, ident) {
}

CodeFragment
OtherType::makeDeclaration(CodeFragment var, const Environment &) {
  return makeTokenNode("/*") + var + makeTokenNode("*/");
}

Type *
OtherType::clone() const {
  OtherType *copy = new OtherType(*this);
  return copy;
}

bool
OtherType::classof(const Type *T) {
  return T->getKind() == TypeKind::Other;
}

OtherType::OtherType(const OtherType &other) : Type(other) {
}

/*!
 * \brief Return the kind of \c type.
 */
TypeKind
typeKind(TypeRef type) {
  return type->getKind();
}

CodeFragment
makeDecl(TypeRef type, CodeFragment var, const Environment &env) {
  if (type) {
    return type->makeDeclaration(cv_qualify(type, var), env);
  } else {
    return makeTokenNode("UNKNOWN_TYPE");
  }
}

TypeRef
makeReservedType(DataTypeIdent ident, CodeFragment name, bool c, bool v) {
  auto type = std::make_shared<Reserved>(ident, name);
  type->setConst(c);
  type->setVolatile(v);
  return type;
}

TypeRef
makeQualifiedType(const DataTypeIdent &ident,
    const DataTypeIdent &underlyingType,
    bool c,
    bool v) {
  return std::make_shared<QualifiedType>(ident, underlyingType, c, v);
}

TypeRef
makePointerType(DataTypeIdent ident, TypeRef ref) {
  return std::make_shared<Pointer>(ident, ref);
}

TypeRef
makePointerType(DataTypeIdent ident, DataTypeIdent ref) {
  return std::make_shared<Pointer>(ident, ref);
}

TypeRef
makeLValueReferenceType(const DataTypeIdent &ident, const DataTypeIdent &ref) {
  return std::make_shared<LValueReferenceType>(ident, ref);
}

TypeRef
makeFunctionType(DataTypeIdent ident,
    TypeRef returnType,
    const Function::Params &params,
    bool isVariadic) {
  return std::make_shared<Function>(ident, returnType, params, isVariadic);
}

TypeRef
makeArrayType(DataTypeIdent ident, TypeRef elemType, size_t size) {
  return std::make_shared<Array>(ident, elemType->dataTypeIdent(), size);
}

TypeRef
makeArrayType(DataTypeIdent ident, DataTypeIdent elemType, size_t size) {
  return std::make_shared<Array>(ident, elemType, size);
}

TypeRef
makeArrayType(DataTypeIdent ident, TypeRef elemType, Array::Size size) {
  return std::make_shared<Array>(ident, elemType->dataTypeIdent(), size);
}

TypeRef
makeArrayType(DataTypeIdent ident, DataTypeIdent elemName, Array::Size size) {
  return std::make_shared<Array>(ident, elemName, size);
}

TypeRef
makeEnumType(const DataTypeIdent &ident) {
  return std::make_shared<EnumType>(ident, EnumType::EnumName());
}

TypeRef
makeStructType(const DataTypeIdent &ident,
    const CodeFragment &tag,
    const Struct::MemberList &fields) {
  return std::make_shared<Struct>(ident, tag, fields);
}

TypeRef
makeClassType(const DataTypeIdent &ident, const ClassType::Symbols &symbols) {
  return std::make_shared<ClassType>(ident, symbols);
}

TypeRef
makeClassType(const DataTypeIdent &ident,
    const std::vector<ClassType::BaseClass> &bases,
    const ClassType::Symbols &symbols) {
  return std::make_shared<ClassType>(ident, bases, symbols);
}

TypeRef
makeOtherType(const DataTypeIdent &ident) {
  return std::make_shared<OtherType>(ident);
}

CodeFragment
TypeRefToString(TypeRef type, const Environment &env) {
  return makeDecl(type, makeTokenNode(""), env);
}

namespace {

template <typename T>
TypeRef
getPointee(const TypeRef &type, const Environment &env) {
  return llvm::cast<T>(type.get())->getPointee(env);
}

} // namespace

bool
hasParen(const TypeRef &type, const Environment &env) {
  return llvm::isa<Function>(type.get());
  switch (type->getKind()) {
  case TypeKind::Function: return true;

  case TypeKind::Array: {
    const auto elemT = llvm::cast<Array>(type.get())->getElemType(env);
    return hasParen(elemT, env);
  }
  case TypeKind::LValueReference:
    return hasParen(getPointee<LValueReferenceType>(type, env), env);
  case TypeKind::Pointer: return hasParen(getPointee<Pointer>(type, env), env);

  default: return false;
  }
}
}
