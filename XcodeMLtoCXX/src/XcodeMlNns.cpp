#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "StringTree.h"
#include "Util.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlTypeTable.h"

using CXXCodeGen::makeTokenNode;

namespace XcodeMl {

Nns::Nns(NnsKind k, const NnsIdent &ni) : parent(), kind(k), ident(ni) {
}

Nns::Nns(NnsKind k, const NnsRef &nr, const NnsIdent &ni)
    : parent(nr ? nr->getParent() : llvm::Optional<NnsIdent>()),
      kind(k),
      ident(ni) {
}

Nns::Nns(NnsKind k, const NnsIdent &par, const NnsIdent &ident)
    : parent(par), kind(k), ident(ident) {
}

Nns::~Nns() = default;

NnsKind
Nns::getKind() const {
  return kind;
}

CodeFragment
Nns::makeDeclaration(const TypeTable &env, const NnsTable &nnss) const {
  const auto par = getParent();
  if (!par.hasValue()) {
    return makeNestedNameSpec(env, nnss);
  }
  const auto p = getOrNull(nnss, *par);
  if (!p.hasValue()) {
    std::cerr << "Undefined NNS: '" << *par << "'" << std::endl;
    std::abort();
  }
  const auto prefix = (*p)->makeDeclaration(env, nnss);
  return prefix + makeNestedNameSpec(env, nnss);
}

llvm::Optional<NnsIdent>
Nns::getParent() const {
  return parent;
}

GlobalNns::GlobalNns() : Nns(NnsKind::Global, NnsRef(), "global") {
}

Nns *
GlobalNns::clone() const {
  GlobalNns *copy = new GlobalNns(*this);
  return copy;
}

bool
GlobalNns::classof(const Nns *N) {
  return N->getKind() == NnsKind::Global;
}

CodeFragment
GlobalNns::makeNestedNameSpec(const TypeTable &, const NnsTable &) const {
  return makeTokenNode("::");
}

llvm::Optional<NnsIdent>
GlobalNns::getParent() const {
  return llvm::Optional<NnsIdent>();
}

ClassNns::ClassNns(
    const NnsIdent &ni, const NnsIdent &parent, const DataTypeIdent &di)
    : Nns(NnsKind::Class, parent, ni), dtident(di) {
}

ClassNns::ClassNns(const NnsIdent &ni, const DataTypeIdent &di)
    : Nns(NnsKind::Class, ni), dtident(di) {
}

Nns *
ClassNns::clone() const {
  ClassNns *copy = new ClassNns(*this);
  return copy;
}

CodeFragment
ClassNns::makeNestedNameSpec(
    const TypeTable &env, const NnsTable &nnsTable) const {
  const auto T = env.at(dtident);
  const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
  assert(classT);
  if (const auto tid = classT->getAsTemplateId(env, nnsTable)) {
    return *tid + makeTokenNode("::");
  }
  const auto name = classT->name();
  return name + makeTokenNode("::");
}

bool
ClassNns::classof(const Nns *N) {
  return N->getKind() == NnsKind::Class;
}

NamespaceNns::NamespaceNns(
    const NnsIdent &nident, const std::string &namespaceName)
    : Nns(NnsKind::Namespace, nident), name(namespaceName) {
}

NamespaceNns::NamespaceNns(const NnsIdent &nident,
    const std::string &namespaceName,
    const NnsIdent &parent)
    : Nns(NnsKind::Namespace, parent, nident), name(namespaceName) {
}

Nns *
NamespaceNns::clone() const {
  NamespaceNns *copy = new NamespaceNns(*this);
  return copy;
}

bool
NamespaceNns::classof(const Nns *N) {
  return N->getKind() == NnsKind::Namespace;
}

CodeFragment
NamespaceNns::makeNestedNameSpec(const TypeTable &, const NnsTable &) const {
  return makeTokenNode(name) + makeTokenNode("::");
}

UnnamedNamespaceNns::UnnamedNamespaceNns(
    const NnsIdent &nident, const NnsIdent &parent)
    : Nns(NnsKind::UnnamedNamespace, parent, nident) {
}

Nns *
UnnamedNamespaceNns::clone() const {
  UnnamedNamespaceNns *copy = new UnnamedNamespaceNns(*this);
  return copy;
}

bool
UnnamedNamespaceNns::classof(const Nns *N) {
  return N->getKind() == NnsKind::UnnamedNamespace;
}

CodeFragment
UnnamedNamespaceNns::makeNestedNameSpec(
    const TypeTable &, const NnsTable &) const {
  return CXXCodeGen::makeVoidNode();
}

OtherNns::OtherNns(const NnsIdent &ident) : Nns(NnsKind::Other, ident) {
}

Nns *
OtherNns::clone() const {
  OtherNns *copy = new OtherNns(*this);
  return copy;
}

bool
OtherNns::classof(const Nns *N) {
  return N->getKind() == NnsKind::Other;
}

CodeFragment
OtherNns::makeNestedNameSpec(const TypeTable &, const NnsTable &) const {
  return CXXCodeGen::makeVoidNode();
}

NnsRef
makeGlobalNns() {
  return std::make_shared<GlobalNns>();
}

NnsRef
makeClassNns(const NnsIdent &ident,
    const NnsIdent &parent,
    const DataTypeIdent &classType) {
  return std::make_shared<ClassNns>(ident, parent, classType);
}

NnsRef
makeClassNns(const NnsIdent &ident, const DataTypeIdent &classType) {
  return std::make_shared<ClassNns>(ident, classType);
}

NnsRef
makeNamespaceNns(const NnsIdent &nident, const std::string &name) {
  return std::make_shared<NamespaceNns>(nident, name);
}

NnsRef
makeUnnamedNamespaceNns(const NnsIdent &nident, const NnsIdent &parent) {
  return std::make_shared<UnnamedNamespaceNns>(nident, parent);
}

NnsRef
makeNamespaceNns(
    const NnsIdent &nident, const NnsIdent &parent, const std::string &name) {
  return std::make_shared<NamespaceNns>(nident, name, parent);
}

NnsRef
makeOtherNns(const NnsIdent &nident) {
  return std::make_shared<OtherNns>(nident);
}

} // namespace XcodeMl
