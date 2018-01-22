#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <libxml/tree.h>
#include "clang/AST/Mangle.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclBase.h"
#include "TypeTableInfo.h"

#include "NnsTableInfo.h"

namespace {

xmlNodePtr getNnsDefElem(const NnsTableInfoImpl &, const std::string &);

void pushNns(NnsTableInfoImpl &, const std::string &);

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
      : mangleContext(MC),
        typetableinfo(TTI),
        mapFromNnsIdentToXmlNodePtr(),
        nnsTableStack(),
        seqForOther(0),
        mapForOtherNns() {
    assert(typetableinfo);
  }

  clang::MangleContext *mangleContext;
  TypeTableInfo *typetableinfo;
  std::map<std::string, xmlNodePtr> mapFromNnsIdentToXmlNodePtr;

  /*! stack of node-NNSs pairs.
   *  Each node-NNSs pair consists
   *  - an XML <nnsTable> element (`xmlNodePtr`)
   *  - and a list of NNSs that belong to the NNS-scope
   *    defined by the <nnsTable> element (`std::vector<std::string>`)
   */
  std::stack<std::tuple<xmlNodePtr, std::vector<std::string>>> nnsTableStack;

  /*! counter for others */
  size_t seqForOther;
  std::map<const clang::NestedNameSpecifier *, std::string> mapForOtherNns;

  std::map<const clang::DeclContext *, std::string> mapForDC;
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
pushNns(NnsTableInfoImpl &info, const std::string &nns) {
  std::get<1>(info.nnsTableStack.top()).push_back(nns);
}

} // namespace

void
NnsTableInfo::pushNnsTableStack(xmlNodePtr nnsTableNode) {
  pimpl->nnsTableStack.push(
      std::make_tuple(nnsTableNode, std::vector<std::string>()));
}

void
NnsTableInfo::popNnsTableStack() {
  assert(!pimpl->nnsTableStack.empty());
  const auto nnsTableNode = std::get<0>(pimpl->nnsTableStack.top());
  const auto nnssInCurScope = std::get<1>(pimpl->nnsTableStack.top());
  for (auto &nns : nnssInCurScope) {
    xmlAddChild(nnsTableNode, getNnsDefElem(*pimpl, nns));
  }
  pimpl->nnsTableStack.pop();
}

namespace {

xmlNodePtr
makeNnsDefNodeForType(NnsTableInfoImpl &info,
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
makeNnsDefNodeForNestedNameSpec(const clang::MangleContext &MC,
    NnsTableInfoImpl &info,
    TypeTableInfo &TTI,
    const clang::NestedNameSpecifier *Spec) {
  assert(Spec);
  using SK = clang::NestedNameSpecifier::SpecifierKind;
  switch (Spec->getKind()) {
  case SK::TypeSpec: return makeNnsDefNodeForType(info, TTI, Spec);

  case SK::Global:
    // do not make Nns definition node for global namespace
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
  info.mapFromNnsIdentToXmlNodePtr[name] = makeNnsDefNodeForNestedNameSpec(
      *(info.mangleContext), info, *info.typetableinfo, NestedNameSpec);
  pushNns(info, name);
}

xmlNodePtr
getNnsDefElem(const NnsTableInfoImpl &info, const std::string &nns) {
  const auto iter = info.mapFromNnsIdentToXmlNodePtr.find(nns);
  assert(iter != info.mapFromNnsIdentToXmlNodePtr.cend());
  return iter->second;
}

} // namespace
