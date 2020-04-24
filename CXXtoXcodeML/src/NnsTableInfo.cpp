#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <libxml/tree.h>
#include "clang/AST/Mangle.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
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
makeClassNnsNode(TypeTableInfo &TTI, const clang::DeclContext &DC) {
  const auto &CRD = llvm::cast<clang::CXXRecordDecl>(DC);
  const auto node = xmlNewNode(nullptr, BAD_CAST "classNNS");

  const auto dtident =
      TTI.getTypeName(clang::QualType(CRD.getTypeForDecl(), 0));
  xmlNewProp(node, BAD_CAST "type", BAD_CAST(dtident.c_str()));

  return node;
}

xmlNodePtr
makeClassTemplateSpecializationNode(
    TypeTableInfo &TTI, const clang::DeclContext &DC) {
  const auto &CTSD = llvm::cast<clang::ClassTemplateSpecializationDecl>(DC);
  const auto node =
      xmlNewNode(nullptr, BAD_CAST "classTemplateSpecializationNNS");

  const auto dtident =
      TTI.getTypeName(clang::QualType(CTSD.getTypeForDecl(), 0));
  xmlNewProp(node, BAD_CAST "type", BAD_CAST(dtident.c_str()));

  return node;
}

xmlNodePtr
makeNamespaceNnsNode(const clang::DeclContext &DC) {
  const auto &ND = llvm::cast<clang::NamespaceDecl>(DC);
  const auto node = xmlNewNode(nullptr, BAD_CAST "namespaceNNS");
  if (ND.isAnonymousNamespace()) {
    xmlNewProp(node, BAD_CAST "is_anonymous", BAD_CAST "1");
    return node;
  }
  const auto name = ND.getDeclName().getAsString();
  xmlNodeAddContent(node, BAD_CAST(name.c_str()));
  return node;
}

xmlNodePtr
nnsNewNode(const clang::MangleContext &MC,
    NnsTableInfoImpl &info,
    TypeTableInfo &TTI,
    const clang::DeclContext &DC) {
  using namespace clang;
  switch (DC.getDeclKind()) {
  case Decl::ClassTemplateSpecialization:
    return makeClassTemplateSpecializationNode(TTI, DC);
  case Decl::CXXRecord: return makeClassNnsNode(TTI, DC);
  case Decl::Namespace: return makeNamespaceNnsNode(DC);
  default: return xmlNewNode(nullptr, BAD_CAST "otherNNS");
  }
}

xmlNodePtr
makeNnsDefNodeForDeclContext(const clang::MangleContext &MC,
    NnsTableInfoImpl &info,
    TypeTableInfo &TTI,
    const clang::DeclContext *DC) {
  assert(DC);
  const auto node = nnsNewNode(MC, info, TTI, *DC);
  xmlNewProp(
      node, BAD_CAST "clang_decl_kind", BAD_CAST(DC->getDeclKindName()));
  xmlNewProp(
      node, BAD_CAST "nns", BAD_CAST(getOrRegisterNnsName(info, DC).c_str()));
  const auto parent = getOrRegisterNnsName(info, DC->getParent());
  xmlNewProp(node, BAD_CAST "parent", BAD_CAST(parent.c_str()));
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
