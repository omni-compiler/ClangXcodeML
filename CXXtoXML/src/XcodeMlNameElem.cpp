#include <libxml/tree.h>
#include "clang/Basic/Builtins.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/Casting.h"
#include "XMLVisitorBase.h"
#include "ClangOperator.h"
#include "ClangUtil.h"
#include "TypeTableInfo.h"
#include "XcodeMlNameElem.h"

using namespace clang;
using namespace llvm;

namespace {

const char*
getNameKind(const NamedDecl* ND) {
  auto FD = dyn_cast<FunctionDecl>(ND);
  if (!FD) {
    return "name";
  }
  if (FD->isOverloadedOperator()) {
    return "operator";
  }
  if (isa<CXXConstructorDecl>(FD)) {
    return "constructor";
  }
  if (isa<CXXDestructorDecl>(FD)) {
    return "destructor";
  }
  return "name";
}

xmlNodePtr
makeIdNode(
    TypeTableInfo& TTI,
    const ValueDecl* VD)
{
  auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
  xmlNewProp(
      idNode,
      BAD_CAST "type",
      BAD_CAST TTI.getTypeName(VD->getType()).c_str());
  return idNode;
}

} // namespace

xmlNodePtr
makeNameNode(
    TypeTableInfo& TTI,
    const NamedDecl* ND)
{
  xmlNodePtr node = nullptr;
  if (auto MD = dyn_cast<CXXMethodDecl>(ND)) {
    node = makeNameNodeForCXXMethodDecl(TTI, MD);
  } else {
    node = xmlNewNode(nullptr, BAD_CAST "name");
    xmlNodeSetContent(node, BAD_CAST (ND->getNameAsString().c_str()));
  }

  xmlNewProp(
      node,
      BAD_CAST "name_kind",
      BAD_CAST getNameKind(ND));
  xmlNewProp(
      node,
      BAD_CAST "fullName",
      BAD_CAST ND->getQualifiedNameAsString().c_str());
  if (ND->isLinkageValid()) {
    const auto FL = ND->getFormalLinkage(),
               LI = ND->getLinkageInternal();
    xmlNewProp(
        node,
        BAD_CAST "linkage",
        BAD_CAST stringifyLinkage(FL));
    if (FL != LI) {
      xmlNewProp(
          node,
          BAD_CAST "clang_linkage_internal",
          BAD_CAST stringifyLinkage(LI));
    }
  } else {
    // should not be executed
    xmlNewProp(
        node,
        BAD_CAST "clang_has_invalid_linkage",
        BAD_CAST "true");
  }

  return node;
}

xmlNodePtr
makeNameNodeForCXXMethodDecl(
    TypeTableInfo&,
    const CXXMethodDecl* MD)
{
  auto nameNode = xmlNewNode(nullptr, BAD_CAST "name");
  if (auto OOK = MD->getOverloadedOperator()) {
    xmlNodeAddContent(
        nameNode,
        BAD_CAST OverloadedOperatorKindToString(OOK, MD->param_size()));
  }
  const auto ident = MD->getIdentifier();
  if (!ident) {
    return nameNode;
  }
  xmlNodeAddContent(nameNode, BAD_CAST ident->getName().data());
  return nameNode;
}

xmlNodePtr
makeIdNodeForCXXMethodDecl(
    TypeTableInfo& TTI,
    const CXXMethodDecl* method)
{
  auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
  xmlNewProp(
      idNode,
      BAD_CAST "type",
      BAD_CAST TTI.getTypeName(method->getType()).c_str());
  auto nameNode = makeNameNodeForCXXMethodDecl(TTI, method);
  xmlAddChild(idNode, nameNode);
  return idNode;
}

xmlNodePtr
makeIdNodeForFieldDecl(
    TypeTableInfo& TTI,
    const FieldDecl* field)
{
  auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
  xmlNewProp(
      idNode,
      BAD_CAST "type",
      BAD_CAST TTI.getTypeName(field->getType()).c_str());
  const auto fieldName = field->getIdentifier();
  if (fieldName) {
    /* Emit only if the field has name.
     * Some field does not have name.
     *  Example: `struct A { int : 0; }; // unnamed bit field`
     */
    auto nameNode = xmlNewChild(
        idNode,
        nullptr,
        BAD_CAST "name",
        BAD_CAST fieldName->getName().data());
    xmlNewProp(
        nameNode,
        BAD_CAST "name_kind",
        BAD_CAST "name");
  }
  return idNode;
}
