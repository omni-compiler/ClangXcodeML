#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <libxml/tree.h>
#include "clang/AST/Mangle.h"
#include "clang/AST/ASTContext.h"
#include "TypeTableInfo.h"

#include "NnsTableInfo.h"

namespace {

xmlNodePtr getNnsNode(
    const NnsTableInfoImpl &, const clang::NestedNameSpecifier *);

void pushNns(NnsTableInfoImpl &, const clang::NestedNameSpecifier *);

void registerNestedNameSpec(
    NnsTableInfoImpl &, const clang::NestedNameSpecifier *);

template <typename T, typename... Ts>
std::unique_ptr<T>
make_unique(Ts &&... params) {
  return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

} // namespace

struct NnsTableInfoImpl {
  explicit NnsTableInfoImpl(clang::MangleContext *MC, TypeTableInfo *TTI)
      : seqForOther(0),
        mangleContext(MC),
        typetableinfo(TTI),
        mapForOtherNns(),
        mapFromNestedNameSpecToXmlNodePtr() {
    assert(typetableinfo);
  }

  size_t seqForOther;
  clang::MangleContext *mangleContext;
  TypeTableInfo *typetableinfo;
  std::map<const clang::NestedNameSpecifier *, std::string> mapForOtherNns;
  std::map<const clang::NestedNameSpecifier *, xmlNodePtr>
      mapFromNestedNameSpecToXmlNodePtr;
  std::stack<std::tuple<xmlNodePtr,
      std::vector<const clang::NestedNameSpecifier *>>> nnsTableStack;
};

NnsTableInfo::NnsTableInfo(clang::MangleContext *MC, TypeTableInfo *TTI)
    : pimpl(make_unique<NnsTableInfoImpl>(MC, TTI)) {
}

NnsTableInfo::~NnsTableInfo() = default;

namespace {

std::string
getOrRegisterNnsName(
    NnsTableInfoImpl &info, const clang::NestedNameSpecifier *NestedNameSpec) {
  using SK = clang::NestedNameSpecifier::SpecifierKind;
  if (NestedNameSpec->getKind() == SK::Global) {
    return "global";
  }

  if (info.mapForOtherNns.count(NestedNameSpec) == 0) {
    registerNestedNameSpec(info, NestedNameSpec);
  }
  return info.mapForOtherNns[NestedNameSpec];
}

} // namespace

std::string
NnsTableInfo::getNnsName(const clang::NestedNameSpecifier *NestedNameSpec) {
  return getOrRegisterNnsName(*pimpl, NestedNameSpec);
}

namespace {

void
pushNns(NnsTableInfoImpl &info, const clang::NestedNameSpecifier *Spec) {
  std::get<1>(info.nnsTableStack.top()).push_back(Spec);
}

} // namespace

void
NnsTableInfo::pushNnsTableStack(xmlNodePtr nnsTableNode) {
  pimpl->nnsTableStack.push(std::make_tuple(
      nnsTableNode, std::vector<const clang::NestedNameSpecifier *>()));
}

void
NnsTableInfo::popNnsTableStack() {
  assert(!pimpl->nnsTableStack.empty());
  const auto nnsTableNode = std::get<0>(pimpl->nnsTableStack.top());
  const auto nnssInCurScope = std::get<1>(pimpl->nnsTableStack.top());
  for (auto &nns : nnssInCurScope) {
    xmlAddChild(nnsTableNode, getNnsNode(*pimpl, nns));
  }
  pimpl->nnsTableStack.pop();
}

namespace {

xmlNodePtr
makeNnsIdentNodeForType(NnsTableInfoImpl &info,
    TypeTableInfo &TTI,
    const clang::NestedNameSpecifier *Spec) {
  assert(Spec);
  const auto T = Spec->getAsType();
  assert(T);

  auto node = xmlNewNode(nullptr, BAD_CAST "classNNS");
  auto dtident = TTI.getTypeName(clang::QualType(T, 0));
  xmlNewProp(node, BAD_CAST "type", BAD_CAST(dtident.c_str()));
  xmlNewProp(node,
      BAD_CAST "nns",
      BAD_CAST(getOrRegisterNnsName(info, Spec).c_str()));

  return node;
}

// clang::NestedNameSpecifier::dump is not a const member fucntion.
void
dump(const clang::NestedNameSpecifier &Spec, const clang::MangleContext &MC) {
  const clang::PrintingPolicy policy(MC.getASTContext().getLangOpts());
  std::string ostr;
  llvm::raw_string_ostream os(ostr);
  Spec.print(os, policy);
  os.flush();
  std::cerr << ostr << std::endl;
}

xmlNodePtr
makeNnsIdentNodeForNestedNameSpec(const clang::MangleContext &MC,
    NnsTableInfoImpl &info,
    TypeTableInfo &TTI,
    const clang::NestedNameSpecifier *Spec) {
  assert(Spec);
  using SK = clang::NestedNameSpecifier::SpecifierKind;
  switch (Spec->getKind()) {
  case SK::TypeSpec: return makeNnsIdentNodeForType(info, TTI, Spec);

  case SK::Global:
    // do not make Nns Identifier node for global namespace
    assert(false);

  case SK::Identifier:
  case SK::Namespace:
  case SK::NamespaceAlias:
  case SK::TypeSpecWithTemplate:
  case SK::Super:
    dump(*Spec, MC);
    // FIXME: unimplemented
    assert(false);
  }
  return nullptr;
}

void
registerNestedNameSpec(
    NnsTableInfoImpl &info, const clang::NestedNameSpecifier *NestedNameSpec) {
  assert(NestedNameSpec);

  using SK = clang::NestedNameSpecifier::SpecifierKind;
  if (NestedNameSpec->getKind() == SK::Global) {
    // no need to for registration
    return;
  }

  const auto prefix = static_cast<std::string>("NNS");
  const auto name = prefix + std::to_string(info.seqForOther++);
  info.mapForOtherNns[NestedNameSpec] = name;
  info.mapFromNestedNameSpecToXmlNodePtr[NestedNameSpec] =
      makeNnsIdentNodeForNestedNameSpec(
          *(info.mangleContext), info, *info.typetableinfo, NestedNameSpec);
  pushNns(info, NestedNameSpec);
}

xmlNodePtr
getNnsNode(
    const NnsTableInfoImpl &info, const clang::NestedNameSpecifier *Spec) {
  const auto iter = info.mapFromNestedNameSpecToXmlNodePtr.find(Spec);
  assert(iter != info.mapFromNestedNameSpecToXmlNodePtr.cend());
  return iter->second;
}

} // namespace
