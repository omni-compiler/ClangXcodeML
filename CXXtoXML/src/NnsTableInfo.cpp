#include <map>
#include <stack>
#include <string>
#include <libxml/tree.h>
#include "clang/AST/Mangle.h"

#include "NnsTableInfo.h"

NnsTableInfo::NnsTableInfo(clang::MangleContext* M):
  seqForOther(0),
  MC(M),
  mapForOtherNns(),
  mapFromNestedNameSpecToXmlNodePtr()
{}

std::string
NnsTableInfo::getNnsName(
    const clang::NestedNameSpecifier* NestedNameSpec)
{
  using SK = clang::NestedNameSpecifier::SpecifierKind;
  if (NestedNameSpec->getKind() == SK::Global) {
    return "global";
  }

  if (mapForOtherNns.count(NestedNameSpec) == 0) {
    registerNestedNameSpec(NestedNameSpec);
  }
  return mapForOtherNns[NestedNameSpec];
}

void
NnsTableInfo::pushNnsTableStack(xmlNodePtr nnsTableNode) {
  nnsTableStack.push(
      std::make_tuple(
        nnsTableNode,
        std::vector<const clang::NestedNameSpecifier*>()));
}

void
NnsTableInfo::popNnsTableStack() {
  assert(!nnsTableStack.empty());
  const auto nnsTableNode = std::get<0>(nnsTableStack.top());
  const auto nnssInCurScope = std::get<1>(nnsTableStack.top());
  for (auto& nns : nnssInCurScope) {
    xmlAddChild(nnsTableNode, getNnsNode(nns));
  }
  nnsTableStack.pop();
}

static xmlNodePtr
makeNnsIdentNodeForType(
    NnsTableInfo& NTI,
    const clang::NestedNameSpecifier* Spec)
{
  assert(Spec);
  using SK = clang::NestedNameSpecifier::SpecifierKind;
  assert(Spec->getKind() == SK::TypeSpec);

  auto node = xmlNewNode(nullptr, BAD_CAST "classNNS");
  if (const auto prefix = Spec->getPrefix()) {
    xmlNewProp(
        node,
        BAD_CAST "nns",
        BAD_CAST (NTI.getNnsName(prefix).c_str()));
  }

  return node;
}

static xmlNodePtr
makeNnsIdentNodeForNestedNameSpec(
    NnsTableInfo& NTI,
    const clang::NestedNameSpecifier* Spec)
{
  assert(Spec);
  using SK = clang::NestedNameSpecifier::SpecifierKind;
  switch(Spec->getKind()) {
    case SK::TypeSpec:
      return makeNnsIdentNodeForType(NTI, Spec);

    case SK::Global:
      // do not make Nns Identifier node for global namespace
      assert(false);

    case SK::Identifier:
    case SK::Namespace:
    case SK::NamespaceAlias:
    case SK::TypeSpecWithTemplate:
    case SK::Super:
      // FIXME: unimplemented
      assert(false);
  }
  return nullptr;
}

void
NnsTableInfo::registerNestedNameSpec(
    const clang::NestedNameSpecifier *NestedNameSpec)
{
  assert(NestedNameSpec);

  using SK = clang::NestedNameSpecifier::SpecifierKind;
  if (NestedNameSpec->getKind() == SK::Global) {
    // no need to for registration
    return;
  }

  const auto prefix = static_cast<std::string>("NNS");
  const auto name =
    prefix +
    std::to_string(seqForOther++);
  mapForOtherNns[NestedNameSpec] = name;
  mapFromNestedNameSpecToXmlNodePtr[NestedNameSpec] =
    makeNnsIdentNodeForNestedNameSpec(*this, NestedNameSpec);
}
