#include <cassert>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include "llvm/ADT/Optional.h"
#include "StringTree.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlName.h"
#include "XcodeMlTypeTable.h"
#include "llvm/Support/Casting.h"

#include <iostream>

using XcodeMl::CodeFragment;
using CXXCodeGen::makeTokenNode;
using CXXCodeGen::makeVoidNode;

namespace {

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

} // namespace

namespace XcodeMl {

MemberDecl::MemberDecl(const DataTypeIdent &d, const CodeFragment &c)
    : dtident(d), name(c), bitfield() {
}

MemberDecl::MemberDecl(const DataTypeIdent &d, const CodeFragment &c, size_t s)
    : dtident(d), name(c), bitfield(s) {
}

CodeFragment
MemberDecl::makeDeclaration(
    const TypeTable &env, const NnsTable &nnsTable) const {
  auto type = env.at(dtident);
  return makeDecl(type, name, env, nnsTable)
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
Reserved::makeDeclaration(
    CodeFragment var, const TypeTable &, const NnsTable &) {
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
QualifiedType::makeDeclaration(
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  const auto T = env[underlying];
  TypeRef QT(T->clone());
  if (isConst) {
    QT->setConst(true);
  }
  if (isVolatile) {
    QT->setVolatile(true);
  }
  return makeDecl(QT, var, env, nnsTable);
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
Pointer::makeDeclaration(
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  auto refType = env[ref];
  if (!refType) {
    return makeTokenNode("INCOMPLETE_TYPE *") + var;
  }
  return makeDecl(
      refType, makeTokenNode("(*") + var + makeTokenNode(")"), env, nnsTable);
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
Pointer::getPointee(const TypeTable &env) const {
  return env.at(ref);
}

Pointer::Pointer(const Pointer &other) : Type(other), ref(other.ref) {
}

MemberPointer::MemberPointer(
    DataTypeIdent dtident, DataTypeIdent p, DataTypeIdent r)
    : Type(TypeKind::MemberPointer, dtident), pointee(p), record(r) {
}

Type *
MemberPointer::clone() const {
  MemberPointer *copy = new MemberPointer(*this);
  return copy;
}

bool
MemberPointer::classof(const Type *T) {
  return T->getKind() == TypeKind::MemberPointer;
}

CodeFragment
MemberPointer::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  const auto classTypeName = makeDecl(
      typeTable.at(record), CXXCodeGen::makeVoidNode(), typeTable, nnsTable);
  const auto innerDecl =
    makeTokenNode("(")+wrapWithXcodeMlIdentity(classTypeName)
    + makeTokenNode("::*") + var + makeTokenNode(")");

  const auto pointeeT = typeTable.at(pointee);
  return makeDecl(pointeeT, innerDecl, typeTable, nnsTable);
}

ReferenceType::ReferenceType(
    const DataTypeIdent &ident, TypeKind kind, const DataTypeIdent &r)
    : Type(kind, ident), ref(r) {
}

ReferenceType::~ReferenceType() = default;

TypeRef
ReferenceType::getPointee(const TypeTable &env) const {
  return env.at(ref);
}

LValueReferenceType::LValueReferenceType(
    const DataTypeIdent &ident, const DataTypeIdent &ref)
    : ReferenceType(ident, TypeKind::LValueReference, ref) {
}

CodeFragment
LValueReferenceType::makeDeclaration(
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  return makeDecl(env.at(ref), makeTokenNode("&") + var, env, nnsTable);
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

RValueReferenceType::RValueReferenceType(
    const DataTypeIdent &ident, const DataTypeIdent &ref)
    : ReferenceType(ident, TypeKind::RValueReference, ref) {
}

CodeFragment
RValueReferenceType::makeDeclaration(
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  return makeDecl(env.at(ref), makeTokenNode("&& ") + var, env, nnsTable);
}

Type *
RValueReferenceType::clone() const {
  RValueReferenceType *copy = new RValueReferenceType(*this);
  return copy;
}

bool
RValueReferenceType::classof(const Type *T) {
  return T->getKind() == TypeKind::RValueReference;
}

ParamList::ParamList(
    const std::vector<DataTypeIdent> &paramTypes, bool ellipsis)
    : dtidents(paramTypes), hasEllipsis(ellipsis) {
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
ParamList::makeDeclaration(const std::vector<CodeFragment> &paramNames,
    const TypeTable &typeTable,
    const NnsTable &nnsTable) const {
  assert(dtidents.size() == paramNames.size());
  std::vector<CodeFragment> decls;
  for (int i = 0, len = dtidents.size(); i < len; ++i) {
    const auto ithType = typeTable.at(dtidents[i]);
    decls.push_back(makeDecl(ithType, paramNames[i], typeTable, nnsTable));
  }
  return CXXCodeGen::join(",", decls)
    + (isVariadic() ?
       (decls.size() ? makeTokenNode(",") :makeVoidNode()) +
       makeTokenNode("..."): makeVoidNode());
}

Function::Function(DataTypeIdent ident,
    const DataTypeIdent &r,
    const std::vector<DataTypeIdent> &p,
    bool v)
    : Type(TypeKind::Function, ident),
      returnValue(r),
      params(p, v),
      defaultArgs(p.size(), makeVoidNode()) {
}

CodeFragment
Function::makeDeclarationWithoutReturnType(CodeFragment var,
    const std::vector<CodeFragment> &args,
    const TypeTable &env,
    const NnsTable &nnsTable) {
  auto decl = var + makeTokenNode("(");
  decl = decl + params.makeDeclaration(args, env, nnsTable);
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
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  return makeDeclarationWithoutReturnType(var, defaultArgs, env, nnsTable);
}

CodeFragment
Function::makeDeclaration(CodeFragment var,
    const std::vector<CodeFragment> &args,
    const TypeTable &env,
    const NnsTable &nnsTable) {
  auto returnType = env.at(returnValue);
  auto decl = makeDeclarationWithoutReturnType(var, args, env, nnsTable);
  return makeDecl(returnType, decl, env, nnsTable);
}

CodeFragment
Function::makeDeclaration(
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  return makeDeclaration(var, defaultArgs, env, nnsTable);
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
Array::makeDeclaration(
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  auto elementType(env[element]);
  if (!elementType) {
    return makeTokenNode("INCOMPLETE_TYPE *") + var;
  }
  if(size.kind != Size::Kind::Integer){
    // element size is Processed from callee
    return makeDecl(elementType, var, env, nnsTable);
  }
  const CodeFragment size_expression =
    makeTokenNode(std::to_string(size.size));

  const CodeFragment declarator = makeTokenNode("[")
      + (isConst() ? makeTokenNode("const") : makeVoidNode())
      + (isVolatile() ? makeTokenNode("volatile") : makeVoidNode())
      + size_expression + makeTokenNode("]");
  return makeDecl(elementType, var + declarator, env, nnsTable);
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
Array::getElemType(const TypeTable &env) const {
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
Struct::makeDeclaration(
    CodeFragment var, const TypeTable &, const NnsTable &) {
  return makeTokenNode("struct") + tag + var;
}

CodeFragment
Struct::makeStructDefinition(
    const TypeTable &env, const NnsTable &nnsTable) const {
  auto body = makeVoidNode();
  for (auto &field : fields) {
    body = body + field.makeDeclaration(env, nnsTable);
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
    : Type(TypeKind::Enum, ident), name_(name) {
}

CodeFragment
EnumType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  const auto nameSpelling =
      name_ ? name_->toString(typeTable, nnsTable) : makeVoidNode();
  return makeTokenNode("enum") + nameSpelling + var;
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

EnumType::EnumName
EnumType::name() const {
  return name_;
}

void
EnumType::setName(const std::string &enum_name) {
  assert(!name_);
  name_ = std::make_shared<UIDIdent>(enum_name);
}

EnumType::EnumType(const EnumType &other) : Type(other), name_(other.name_) {
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
UnionType::makeDeclaration(
    CodeFragment var, const TypeTable &env, const NnsTable &nnsTable) {
  auto memberDecls = makeVoidNode();
  for (auto &member : members) {
    memberDecls = memberDecls + member.makeDeclaration(env, nnsTable);
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
      classKind_(CXXClassKind::Class),
      name_(className),
      bases_(),
      classScopeSymbols(symbols),
      templateArgs()
{
}

ClassType::ClassType(const DataTypeIdent &ident,
    CXXClassKind kind,
    const llvm::Optional<std::string> &nns,
    const CodeFragment &className,
    const std::vector<BaseClass> &b,
    const ClassType::Symbols &symbols,
		     const llvm::Optional<TemplateArgList> &argList,
		     uintptr_t n
		     )
    : Type(TypeKind::Class, ident),
      classKind_(kind),
      nnsident(nns),
      name_(className),
      bases_(b),
      classScopeSymbols(symbols),
      templateArgs(argList),node(n)
      
{
}

ClassType::ClassType(
    const DataTypeIdent &ident, const ClassType::Symbols &symbols)
    : Type(TypeKind::Class, ident),
      classKind_(CXXClassKind::Class),
      name_(),
      bases_(),
      classScopeSymbols(symbols),
      templateArgs(),node(0)
{
}

std::string
getClassKey(CXXClassKind kind) {
  switch (kind) {
  case CXXClassKind::Class: return "class";
  case CXXClassKind::Struct: return "struct";
  case CXXClassKind::Union: return "union";
  }
}

CodeFragment
ClassType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  if (!nnsident.hasValue()) {
    assert(name_);
    if (const auto tid = getAsTemplateId(typeTable, nnsTable)) {
      return makeTokenNode(getClassKey(classKind())) + *tid + var;
    }
    return makeTokenNode(getClassKey(classKind())) + name_ + var;
  }
  const auto nns = nnsTable.at(*nnsident);
  const auto spec = nns->makeDeclaration(typeTable, nnsTable);
  if (const auto tid = getAsTemplateId(typeTable, nnsTable)) {
    return makeTokenNode(getClassKey(classKind())) + spec + *tid + var;
  }
  return makeTokenNode(getClassKey(classKind())) + spec + name_ + var;
}

Type *
ClassType::clone() const {
  ClassType *copy = new ClassType(*this);
  return copy;
}

CXXClassKind
ClassType::classKind() const {
  return classKind_;
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
ClassType::isClassTemplateSpecialization() const {
  return templateArgs.hasValue();
}

llvm::Optional<CodeFragment>
ClassType::getAsTemplateId(
    const TypeTable &typeTable, const NnsTable &nnsTable) const {
  using MaybeCodeFragment = llvm::Optional<CodeFragment>;
  if (!templateArgs.hasValue()) {
    return MaybeCodeFragment();
  }
  std::vector<CodeFragment> targs;
  for (auto &&dtident : *templateArgs) {
    CodeFragment arg;
    if(!dtident.argType){
      const auto T = typeTable.at(dtident.ident);
      arg = makeDecl(T, makeVoidNode(), typeTable, nnsTable);
    }else{
      arg = makeTokenNode(dtident.ident);
    }
    targs.push_back(arg);
  }
  const auto list = makeTokenNode("<") + join(",", targs) + makeTokenNode(">");
  return MaybeCodeFragment(name_ + list);
}

bool
ClassType::classof(const Type *T) {
  return T->getKind() == TypeKind::Class;
}

ClassType::ClassType(const ClassType &other)
    : Type(other),
      classKind_(other.classKind_),
      name_(other.name_),
      classScopeSymbols(other.classScopeSymbols) {
}

TemplateTypeParm::TemplateTypeParm(
	   const DataTypeIdent &dtident, const CodeFragment &name, int p )
  : Type(TypeKind::TemplateTypeParm, dtident), pSpelling(name), pack(p) {
}

CodeFragment
TemplateTypeParm::makeDeclaration(
    CodeFragment var, const TypeTable &, const NnsTable &) {
  assert(pSpelling.hasValue());
  auto packsuffix = (pack) ?  makeTokenNode("..."): makeVoidNode();
  return  (*pSpelling) + var + packsuffix;
}

Type *
TemplateTypeParm::clone() const {
  TemplateTypeParm *copy = new TemplateTypeParm(*this);
  return copy;
}

TemplateTypeParm::TemplateTypeParm(const TemplateTypeParm &other)
  : Type(other), pSpelling(other.pSpelling), pack(other.pack) {
}

bool
TemplateTypeParm::classof(const Type *T) {
  return T->getKind() == TypeKind::TemplateTypeParm;
}

void
TemplateTypeParm::setSpelling(CodeFragment T) {
  pSpelling = T;
}

llvm::Optional<CodeFragment>
TemplateTypeParm::getSpelling() const {
  using MaybeName = llvm::Optional<CodeFragment>;
  if (!pSpelling) {
    return MaybeName();
  }
  return pSpelling;
}
TemplateSpecializationType::TemplateSpecializationType(const DataTypeIdent &ident, const CodeFragment &name_, const llvm::Optional<TemplateArgList> &Ta)
    : Type(TypeKind::TemplateSpecialization, ident), name(name_),  templateArgs(Ta){
}

CodeFragment
TemplateSpecializationType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  std::vector<CodeFragment> targs;
  for (auto &&dtident : *templateArgs) {
    CodeFragment arg;
    if(!dtident.argType){
      const auto T = typeTable.at(dtident.ident);
      arg = makeDecl(T, makeVoidNode(), typeTable, nnsTable);
    }else{
      arg = makeTokenNode(dtident.ident);
    }
    targs.push_back(arg);
  }
  const auto list = makeTokenNode("<") + join(",", targs) + makeTokenNode(">");

  return name + list;
}
Type *
TemplateSpecializationType::clone() const {
  TemplateSpecializationType *copy
    = new TemplateSpecializationType(*this);
  return copy;
}

bool
TemplateSpecializationType::classof(const Type *T) {
  return T->getKind() == TypeKind::TemplateSpecialization;
}

DependentTemplateSpecializationType::DependentTemplateSpecializationType(const DataTypeIdent &ident/*, const CodeFragment &name_, const llvm::Optional<TemplateArgList> &Ta*/)
  : Type(TypeKind::DependentTemplateSpecialization, ident)/*, name(name_),  templateArgs(Ta)*/{
}

CodeFragment
DependentTemplateSpecializationType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
#if 0
  std::vector<CodeFragment> targs;
  for (auto &&dtident : *templateArgs) {
    CodeFragment arg;
    if(!dtident.argType){
      const auto T = typeTable.at(dtident.ident);
      arg = makeDecl(T, makeVoidNode(), typeTable, nnsTable);
    }else{
      arg = makeTokenNode(dtident.ident);
    }
    targs.push_back(arg);
  }
  const auto list = makeTokenNode("<") + join(",", targs) + makeTokenNode(">");

  return name + list;
#endif
  return makeTokenNode("/*DependentTemplateSpesialization*/");
}

Type *
DependentTemplateSpecializationType::clone() const {
  DependentTemplateSpecializationType *copy
    = new DependentTemplateSpecializationType(*this);
  return copy;
}

bool
DependentTemplateSpecializationType::classof(const Type *T) {
  return T->getKind() == TypeKind::DependentTemplateSpecialization;
}
DependentNameType::DependentNameType(const DataTypeIdent &ident,const DataTypeIdent &u,const DataTypeIdent &m)
  : Type(TypeKind::DependentName, ident),upper(u),member(m)
{
}
DependentNameType::DependentNameType(const DependentNameType &dn) : Type(dn), upper(dn.upper),member(dn.member)
{
}

CodeFragment
DependentNameType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  const auto T = typeTable.at(upper);
  return makeTokenNode("typename ")+T->makeDeclaration(makeVoidNode(),typeTable,nnsTable)+makeTokenNode("::") + makeTokenNode(member) + var;
}
Type *
DependentNameType::clone() const {
  DependentNameType *copy = new DependentNameType(*this);
  return copy;
}

bool
DependentNameType::classof(const Type *T) {
  return T->getKind() == TypeKind::DependentName;
}

PackExpansionType::PackExpansionType(const PackExpansionType &packex) :
  Type(packex),pattern(packex.pattern) {
}
PackExpansionType::PackExpansionType(const DataTypeIdent &ident, const DataTypeIdent &pat)
  : Type(TypeKind::PackExpansion, ident),pattern(pat) {
}
CodeFragment
PackExpansionType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  const auto T = typeTable.at(pattern);
  return T->makeDeclaration(makeVoidNode(),typeTable,nnsTable)+var;
}

Type *
PackExpansionType::clone() const{
  auto *copy = new PackExpansionType(*this);
  return copy;
}

bool
PackExpansionType::classof(const Type *T) {
  return T->getKind() == TypeKind::PackExpansion;
}

UnaryTransformType::UnaryTransformType(const UnaryTransformType &packex) :
  Type(packex) {
}
UnaryTransformType::UnaryTransformType(const DataTypeIdent &ident, const DataTypeIdent &uident)
  : Type(TypeKind::UnaryTransform, ident), utype(uident) {
}
CodeFragment
UnaryTransformType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  const auto T = typeTable.at(utype);
  return  makeTokenNode("__underlying_type(")+
    T->makeDeclaration(makeVoidNode(),typeTable,nnsTable) +
    makeTokenNode(") ") + var;
}

Type *
UnaryTransformType::clone() const{
  auto *copy = new UnaryTransformType(*this);
  return copy;
}

bool
UnaryTransformType::classof(const Type *T) {
  return T->getKind() == TypeKind::UnaryTransform;
}


AtomicType::AtomicType(const AtomicType &attype) : Type(attype) {
}
AtomicType::AtomicType(const DataTypeIdent &ident, const DataTypeIdent &vtype)
  : Type(TypeKind::Atomic, ident),valuetype(vtype) {
}
CodeFragment
AtomicType::makeDeclaration(
    CodeFragment var, const TypeTable &typeTable, const NnsTable &nnsTable) {
  const auto T = typeTable.at(valuetype);
  return makeTokenNode("_Atomic(") +
    T->makeDeclaration(makeVoidNode(),typeTable,nnsTable) +
    makeTokenNode(") ") + var;
}

Type *
AtomicType::clone() const{
  AtomicType *copy = new AtomicType(*this);
  return copy;
}

bool
AtomicType::classof(const Type *T) {
  return T->getKind() == TypeKind::Atomic;
}


OtherType::OtherType(const OtherType &other) : Type(other) {
}
 
OtherType::OtherType(const DataTypeIdent &ident)
    : Type(TypeKind::Other, ident) {
}

CodeFragment
OtherType::makeDeclaration(
    CodeFragment var, const TypeTable &, const NnsTable &) {
  return makeTokenNode("void") + makeTokenNode("/*") + var + makeTokenNode("*/");
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


DeclType::DeclType(const DeclType &declt) : Type(declt) {
}

DeclType::DeclType(const DataTypeIdent &ident)
    : Type(TypeKind::DeclType, ident) {
}

CodeFragment
DeclType::makeDeclaration(
    CodeFragment var, const TypeTable &, const NnsTable &) {
  return makeTokenNode("decltype (") + var + makeTokenNode(")");
}

Type *
DeclType::clone() const {
  DeclType *copy = new DeclType(*this);
  return copy;
}

bool
DeclType::classof(const Type *T) {
  return T->getKind() == TypeKind::DeclType;
}

/*!
 * \brief Return the kind of \c type.
 */
TypeKind
typeKind(TypeRef type) {
  return type->getKind();
}

CodeFragment
makeDecl(TypeRef type,
    CodeFragment var,
    const TypeTable &env,
    const NnsTable &nnsTable) {
  if (type) {
    return type->makeDeclaration(cv_qualify(type, var), env, nnsTable);
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
makeMemberPointerType(
    DataTypeIdent ident, DataTypeIdent pointee, DataTypeIdent record) {
  return std::make_shared<MemberPointer>(ident, pointee, record);
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
makeRValueReferenceType(const DataTypeIdent &ident, const DataTypeIdent &ref) {
  return std::make_shared<RValueReferenceType>(ident, ref);
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
makeFunctionType(const DataTypeIdent &ident,
    const DataTypeIdent &returnType,
    const std::vector<DataTypeIdent> &paramTypes) {
  return std::make_shared<Function>(ident, returnType, paramTypes);
}

TypeRef
makeFunctionType(const DataTypeIdent &ident,
    const TypeRef &returnType,
    const std::vector<DataTypeIdent> &paramTypes) {
  return std::make_shared<Function>(
      ident, returnType->dataTypeIdent(), paramTypes);
}

TypeRef
makeVariadicFunctionType(const DataTypeIdent &ident,
    const DataTypeIdent &returnType,
    const std::vector<DataTypeIdent> &paramTypes) {
  return std::make_shared<Function>(ident, returnType, paramTypes, true);
}

TypeRef
makeEnumType(
    const DataTypeIdent &ident, const std::shared_ptr<UnqualId> tagname) {
  const auto name = static_cast<EnumType::EnumName>(tagname->clone());
  return std::make_shared<EnumType>(ident, name);
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
makeClassType(const DataTypeIdent &dtident,
    const llvm::Optional<std::string> nnsident,
    const CodeFragment &className,
    const std::vector<ClassType::BaseClass> &bases,
    const ClassType::Symbols &members,
	      const llvm::Optional<TemplateArgList> &targs,
	      const xmlNodePtr node) {

  return std::make_shared<ClassType>(dtident,
      CXXClassKind::Class,
      nnsident,
      className,
      bases,
      members,
      targs,
      reinterpret_cast<uintptr_t>(node)
     );
}

TypeRef
makeCXXUnionType(const DataTypeIdent &ident,
    const llvm::Optional<std::string> nnsident,
    const CodeFragment &unionName,
    const std::vector<ClassType::BaseClass> &bases,
    const ClassType::Symbols &members,
    const llvm::Optional<TemplateArgList> &targs,
    const xmlNodePtr node) {
  return std::make_shared<ClassType>(
	 ident, CXXClassKind::Union, nnsident, unionName, bases, members,
	 targs, reinterpret_cast<uintptr_t>(node));
}

TypeRef
makeTemplateTypeParm(const DataTypeIdent &dtident, const CodeFragment &name, int pack) {
  return std::make_shared<TemplateTypeParm>(dtident, name, pack);
}

TypeRef
makeDependentTemplateSpecializationType(const DataTypeIdent &dtident) {
  return std::make_shared<DependentTemplateSpecializationType>(dtident);
}
TypeRef
makePackExpansionType(const DataTypeIdent &dtident, const DataTypeIdent &pattern) {
  return std::make_shared<PackExpansionType>(dtident, pattern);
}
TypeRef
makeUnaryTransformType(const DataTypeIdent &dtident, const DataTypeIdent &uident)
{
  return std::make_shared<UnaryTransformType>(dtident, uident);
}
TypeRef
makeAtomicType(const DataTypeIdent &dtident, const DataTypeIdent &vident)
{
  return std::make_shared<AtomicType>(dtident, vident);
}

TypeRef
makeTemplateSpecializationType(const DataTypeIdent &dtident, const CodeFragment
			       &name, const llvm::Optional<TemplateArgList>&Ta)
{
  return std::make_shared<TemplateSpecializationType>(dtident, name, Ta);
}
TypeRef
makeOtherType(const DataTypeIdent &ident) {
  return std::make_shared<OtherType>(ident);
}
TypeRef
makeDeclType(const DataTypeIdent &ident){
  return std::make_shared<DeclType>(ident);
}
TypeRef
makeDependentNameType(const DataTypeIdent &ident, const DataTypeIdent &dependtype, const DataTypeIdent &member) {
  return std::make_shared<DependentNameType>(ident, dependtype, member);
}
CodeFragment
TypeRefToString(TypeRef type, const TypeTable &env, const NnsTable &nnsTable) {
  return makeDecl(type, makeTokenNode(""), env, nnsTable);
}

namespace {

template <typename T>
TypeRef
getPointee(const TypeRef &type, const TypeTable &env) {
  return llvm::cast<T>(type.get())->getPointee(env);
}

} // namespace

bool
hasParen(const TypeRef &type, const TypeTable &env) {
  return llvm::isa<Function>(type.get());
  switch (type->getKind()) {
  case TypeKind::Function: return true;

  case TypeKind::Array: {
    const auto elemT = llvm::cast<Array>(type.get())->getElemType(env);
    return hasParen(elemT, env);
  }
  case TypeKind::LValueReference:
    return hasParen(getPointee<LValueReferenceType>(type, env), env);
  case TypeKind::RValueReference:
    return hasParen(getPointee<RValueReferenceType>(type, env), env);
  case TypeKind::Pointer: return hasParen(getPointee<Pointer>(type, env), env);

  default: return false;
  }
}
}
