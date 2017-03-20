#include <map>
#include <memory>
#include <string>
#include <vector>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "StringTree.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"

#include "XcodeMlNns.h"

using CXXCodeGen::makeTokenNode;

namespace XcodeMl {

Nns::Nns(NnsKind k, const NnsRef& nr, const NnsIdent& ni):
  parent(nr),
  kind(k),
  ident(ni)
{}

Nns::~Nns() = default;

NnsKind
Nns::getKind() const {
  return kind;
}

NnsRef
Nns::getParent() const {
  return parent;
}

ClassNns::ClassNns(
    const NnsIdent& ni,
    const NnsRef& parent,
    const DataTypeIdent& di):
  Nns(NnsKind::Class, parent, ni),
  dtident(di)
{}

Nns*
ClassNns::clone() const {
  ClassNns *copy = new ClassNns(*this);
  return copy;
}

CodeFragment
ClassNns::makeNestedNameSpec(
    const Environment& env) const
{
  const auto T = env.at(dtident);
  const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
  assert(classT);
  const auto name = classT->name();
  assert(name.hasValue());
  return (*name) + makeTokenNode("::");
}

bool
ClassNns::classof(const Nns* N) {
  return N->getKind() == NnsKind::Class;
}

} // namespace XcodeMl
