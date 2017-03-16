#include <map>
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
}
