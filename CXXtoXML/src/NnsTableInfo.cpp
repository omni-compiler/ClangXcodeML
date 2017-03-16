#include <map>
#include <string>
#include "clang/AST/Mangle.h"

#include "NnsTableInfo.h"

NnsTableInfo::NnsTableInfo(clang::MangleContext* M):
  seqForOther(0),
  MC(M),
  mapForOtherNns()
{}

std::string
NnsTableInfo::getNnsName(clang::NestedNameSpecifier* NestedNameSpec) {
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
NnsTableInfo::registerNestedNameSpec(
    clang::NestedNameSpecifier *NestedNameSpec)
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
