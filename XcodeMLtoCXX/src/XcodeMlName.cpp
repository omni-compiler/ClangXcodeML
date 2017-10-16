#include <map>
#include <memory>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "StringTree.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "XcodeMlNns.h"
#include "XcodeMlOperator.h"

#include "XcodeMlName.h"

using XcodeMl::CodeFragment;
using CXXCodeGen::makeTokenNode;
using CXXCodeGen::makeVoidNode;

namespace XcodeMl {

UnqualId::UnqualId(UnqualIdKind k) : kind(k) {
}

UnqualId::~UnqualId() {
}

UnqualIdKind
UnqualId::getKind() const {
  return kind;
}

UIDIdent::UIDIdent(const std::string &id)
    : UnqualId(UnqualIdKind::Ident), ident(id) {
}

UnqualId *
UIDIdent::clone() const {
  auto copy = new UIDIdent(*this);
  return copy;
}

CodeFragment
UIDIdent::toString(const Environment &) const {
  return makeTokenNode(ident);
}

bool
UIDIdent::classof(const UnqualId *id) {
  return id->getKind() == UnqualIdKind::Ident;
}

OpFuncId::OpFuncId(const std::string &op)
    : UnqualId(UnqualIdKind::OpFuncId), opName(op) {
}

UnqualId *
OpFuncId::clone() const {
  auto copy = new OpFuncId(*this);
  return copy;
}

CodeFragment
OpFuncId::toString(const Environment &) const {
  const auto op = OperatorNameToSpelling(opName);
  assert(op.hasValue());
  return makeTokenNode("operator") + makeTokenNode(*op);
}

bool
OpFuncId::classof(const UnqualId *id) {
  return id->getKind() == UnqualIdKind::OpFuncId;
}

ConvFuncId::ConvFuncId(const DataTypeIdent &type)
    : UnqualId(UnqualIdKind::ConvFuncId), dtident(type) {
}

UnqualId *
ConvFuncId::clone() const {
  auto copy = new ConvFuncId(*this);
  return copy;
}

CodeFragment
ConvFuncId::toString(const Environment &env) const {
  const auto T = env.at(dtident);
  return makeTokenNode("operator") + T->makeDeclaration(makeVoidNode(), env);
}

bool
ConvFuncId::classof(const UnqualId *id) {
  return id->getKind() == UnqualIdKind::ConvFuncId;
}

CtorName::CtorName(const DataTypeIdent &d)
    : UnqualId(UnqualIdKind::Ctor), dtident(d) {
}

UnqualId *
CtorName::clone() const {
  auto copy = new CtorName(*this);
  return copy;
}

CodeFragment
CtorName::toString(const Environment &env) const {
  const auto T = env.at(dtident);
  return T->makeDeclaration(makeVoidNode(), env);
}

bool
CtorName::classof(const UnqualId *id) {
  return id->getKind() == UnqualIdKind::Ctor;
}

DtorName::DtorName(const DataTypeIdent &d)
    : UnqualId(UnqualIdKind::Dtor), dtident(d) {
}

UnqualId *
DtorName::clone() const {
  auto copy = new DtorName(*this);
  return copy;
}

CodeFragment
DtorName::toString(const Environment &env) const {
  const auto T = env.at(dtident);
  return makeTokenNode("~") + T->makeDeclaration(makeVoidNode(), env);
}

bool
DtorName::classof(const UnqualId *id) {
  return id->getKind() == UnqualIdKind::Ctor;
}

} // namespace XcodeMl
