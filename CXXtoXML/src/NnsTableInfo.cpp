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

void registerDeclContext(NnsTableInfoImpl &, const clang::DeclContext *DC);

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
        mapForDC() {
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

  std::map<const clang::DeclContext *, std::string> mapForDC;
};

NnsTableInfo::NnsTableInfo(clang::MangleContext *MC, TypeTableInfo *TTI)
    : pimpl(make_unique<NnsTableInfoImpl>(MC, TTI)) {
}

NnsTableInfo::~NnsTableInfo() = default;

namespace {

std::string
getOrRegisterNnsName(NnsTableInfoImpl &info, const clang::DeclContext *DC) {
  using namespace clang;

  const auto kind = DC->getDeclKind();
  if (kind == Decl::TranslationUnit) {
    return "global";
  }

  if (info.mapForDC.count(DC) == 0) {
    registerDeclContext(info, DC);
  }
  return info.mapForDC[DC];
}

} // namespace

std::string
NnsTableInfo::getNnsName(const clang::DeclContext *DC) {
  return getOrRegisterNnsName(*pimpl, DC);
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
makeNnsDefNodeForDeclContext(const clang::MangleContext &MC,
    NnsTableInfoImpl &info,
    TypeTableInfo &TTI,
    const clang::DeclContext *DC) {
  using namespace clang;
  assert(DC);
  const auto node = xmlNewNode(nullptr, BAD_CAST "DCNNS");
  xmlNewProp(node, BAD_CAST "kind", BAD_CAST(DC->getDeclKindName()));
  xmlNewProp(
      node, BAD_CAST "nns", BAD_CAST(getOrRegisterNnsName(info, DC).c_str()));
  return node;
}

void
registerDeclContext(NnsTableInfoImpl &info, const clang::DeclContext *DC) {
  assert(DC);

  if (DC->getDeclKind() == clang::Decl::TranslationUnit) {
    // no need to register
    return;
  }
  const auto prefix = static_cast<std::string>("NNS");
  const auto name = prefix + std::to_string(info.seqForOther++);
  info.mapForDC[DC] = name;
  info.mapFromNnsIdentToXmlNodePtr[name] = makeNnsDefNodeForDeclContext(
      *(info.mangleContext), info, *info.typetableinfo, DC);
  pushNns(info, name);
}

xmlNodePtr
getNnsDefElem(const NnsTableInfoImpl &info, const std::string &nns) {
  const auto iter = info.mapFromNnsIdentToXmlNodePtr.find(nns);
  assert(iter != info.mapFromNnsIdentToXmlNodePtr.cend());
  return iter->second;
}

} // namespace
