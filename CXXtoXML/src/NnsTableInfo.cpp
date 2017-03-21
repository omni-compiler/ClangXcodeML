#include <map>
#include <stack>
#include <string>
#include <libxml/tree.h>
#include "clang/AST/Mangle.h"
#include "TypeTableInfo.h"

#include "NnsTableInfo.h"

NnsTableInfo::NnsTableInfo(
    TypeTableInfo* TTI):
  seqForOther(0),
  typetableinfo(TTI),
  mapForOtherNns(),
  mapFromNestedNameSpecToXmlNodePtr()
{
  assert(typetableinfo);
}

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
NnsTableInfo::pushNns(const clang::NestedNameSpecifier* Spec) {
  std::get<1>(nnsTableStack.top()).push_back(Spec);
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
    TypeTableInfo& TTI,
    const clang::NestedNameSpecifier* Spec)
{
  assert(Spec);
  const auto T = Spec->getAsType();
  assert(T);

  auto node = xmlNewNode(nullptr, BAD_CAST "classNNS");
  auto dtident = TTI.getTypeName(
      clang::QualType(T, 0));
  xmlNewProp(
      node,
      BAD_CAST "type",
      BAD_CAST (dtident.c_str()));
  xmlNewProp(
      node,
      BAD_CAST "nns",
      BAD_CAST (NTI.getNnsName(Spec).c_str()));

  return node;
}

static xmlNodePtr
makeNnsIdentNodeForNestedNameSpec(
    NnsTableInfo& NTI,
    TypeTableInfo& TTI,
    const clang::NestedNameSpecifier* Spec)
{
  assert(Spec);
  using SK = clang::NestedNameSpecifier::SpecifierKind;
  switch(Spec->getKind()) {
    case SK::TypeSpec:
      return makeNnsIdentNodeForType(NTI, TTI, Spec);

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
    makeNnsIdentNodeForNestedNameSpec(
        *this,
        *typetableinfo,
        NestedNameSpec);
  pushNns(NestedNameSpec);
}

xmlNodePtr
NnsTableInfo::getNnsNode(const clang::NestedNameSpecifier* Spec) const {
  auto iter = mapFromNestedNameSpecToXmlNodePtr.find(Spec);
  assert(iter != mapFromNestedNameSpecToXmlNodePtr.end());
  return iter->second;
}
