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

const char *
getNameKind(const NamedDecl *ND) {
  const auto DN = ND->getDeclName();

  using NK = clang::DeclarationName::NameKind;
  switch (DN.getNameKind()) {
  case NK::Identifier: return "name";
  case NK::CXXOperatorName: return "operator";
  case NK::CXXConversionFunctionName: return "conversion";
  case NK::CXXConstructorName: return "constructor";
  case NK::CXXDestructorName: return "destructor";
  default: assert(false && "not supported");
  }
}

xmlNodePtr
makeIdNode(TypeTableInfo &TTI, const ValueDecl *VD) {
  auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
  xmlNewProp(idNode,
      BAD_CAST "type",
      BAD_CAST TTI.getTypeName(VD->getType()).c_str());
  return idNode;
}

void
xmlNewBoolProp(xmlNodePtr node, const std::string &name, bool value) {
  xmlNewProp(node, BAD_CAST(name.c_str()), BAD_CAST(value ? "true" : "false"));
}

xmlNodePtr
makeCtorNode(TypeTableInfo &TTI, const CXXConstructorDecl *ctor) {
  auto node = xmlNewNode(nullptr, BAD_CAST "name");
  xmlNewBoolProp(node, "is_implicit", ctor->isExplicit());
  xmlNewBoolProp(node, "is_copy_constructor", ctor->isCopyConstructor());
  xmlNewBoolProp(node, "is_move_constructor", ctor->isMoveConstructor());

  const auto T = ctor->getDeclName().getCXXNameType();
  xmlNewProp(node, BAD_CAST "ctor_type", BAD_CAST TTI.getTypeName(T).c_str());

  return node;
}

xmlNodePtr
makeDtorNode(TypeTableInfo &TTI, const CXXDestructorDecl *dtor) {
  auto node = xmlNewNode(nullptr, BAD_CAST "name");
  xmlNewBoolProp(node, "is_implicit", dtor->isImplicit());

  const auto T = dtor->getDeclName().getCXXNameType();
  xmlNewProp(node, BAD_CAST "dtor_type", BAD_CAST TTI.getTypeName(T).c_str());

  return node;
}

xmlNodePtr
makeConvNode(TypeTableInfo &TTI, const CXXConversionDecl *conv) {
  auto node = xmlNewNode(nullptr, BAD_CAST "name");

  const auto convT = conv->getConversionType();
  xmlNewProp(node,
      BAD_CAST "destination_type",
      BAD_CAST TTI.getTypeName(convT).c_str());

  return node;
}

xmlNodePtr
makeNameNodeForCXXOperator(TypeTableInfo &, const FunctionDecl *FD) {
  assert(FD->getOverloadedOperator() != OO_None);
  const auto opName = getOperatorString(FD);
  auto opNode = xmlNewNode(nullptr, BAD_CAST "name");
  xmlNodeAddContent(opNode, BAD_CAST opName);
  return opNode;
}

xmlNodePtr
makeNameNodeForCXXMethodDecl(TypeTableInfo &TTI, const CXXMethodDecl *MD) {
  // Assume `MD` is not an overloaded operator.
  // (makeNameNodeForCXXOperator handles them)
  using NK = clang::DeclarationName::NameKind;
  assert(MD->getDeclName().getNameKind() != NK::CXXOperatorName);

  if (const auto ctor = dyn_cast<CXXConstructorDecl>(MD)) {
    return makeCtorNode(TTI, ctor);
  } else if (const auto dtor = dyn_cast<CXXDestructorDecl>(MD)) {
    return makeDtorNode(TTI, dtor);
  } else if (const auto conv = dyn_cast<CXXConversionDecl>(MD)) {
    return makeConvNode(TTI, conv);
  }

  auto nameNode = xmlNewNode(nullptr, BAD_CAST "name");
  const auto ident = MD->getIdentifier();
  if (!ident) {
    return nameNode;
  }
  xmlNodeAddContent(nameNode, BAD_CAST ident->getName().data());
  return nameNode;
}

} // namespace

xmlNodePtr
makeNameNode(TypeTableInfo &TTI, const NamedDecl *ND) {
  xmlNodePtr node = nullptr;
  using NK = clang::DeclarationName::NameKind;
  if (ND->getDeclName().getNameKind() == NK::CXXOperatorName) {
    // An overloaded operator can be a member function
    // or a non-member function (= CXXMethod).
    const auto FD = cast<FunctionDecl>(ND);
    node = makeNameNodeForCXXOperator(TTI, FD);
  } else if (auto MD = dyn_cast<CXXMethodDecl>(ND)) {
    node = makeNameNodeForCXXMethodDecl(TTI, MD);
  } else {
    node = xmlNewNode(nullptr, BAD_CAST "name");
    xmlNodeSetContent(node, BAD_CAST(ND->getNameAsString().c_str()));
  }

  xmlNewProp(node, BAD_CAST "name_kind", BAD_CAST getNameKind(ND));
  xmlNewProp(node,
      BAD_CAST "fullName",
      BAD_CAST ND->getQualifiedNameAsString().c_str());
  if (ND->isLinkageValid()) {
    const auto FL = ND->getFormalLinkage(), LI = ND->getLinkageInternal();
    xmlNewProp(node, BAD_CAST "linkage", BAD_CAST stringifyLinkage(FL));
    if (FL != LI) {
      xmlNewProp(node,
          BAD_CAST "clang_linkage_internal",
          BAD_CAST stringifyLinkage(LI));
    }
  } else {
    // should not be executed
    xmlNewProp(node, BAD_CAST "clang_has_invalid_linkage", BAD_CAST "true");
  }

  return node;
}

xmlNodePtr
makeNameNode(TypeTableInfo &TTI, const DeclRefExpr *DRE) {
  return makeNameNode(TTI, DRE->getFoundDecl());
}

xmlNodePtr
makeIdNodeForCXXMethodDecl(TypeTableInfo &TTI, const CXXMethodDecl *method) {
  auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
  xmlNewProp(idNode,
      BAD_CAST "type",
      BAD_CAST TTI.getTypeName(method->getType()).c_str());
  auto nameNode = makeNameNode(TTI, method);
  xmlAddChild(idNode, nameNode);
  return idNode;
}

xmlNodePtr
makeIdNodeForFieldDecl(TypeTableInfo &TTI, const FieldDecl *field) {
  auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
  xmlNewProp(idNode,
      BAD_CAST "type",
      BAD_CAST TTI.getTypeName(field->getType()).c_str());
  const auto fieldName = field->getIdentifier();
  if (fieldName) {
    /* Emit only if the field has name.
     * Some field does not have name.
     *  Example: `struct A { int : 0; }; // unnamed bit field`
     */
    auto nameNode = makeNameNode(TTI, field);
    xmlAddChild(idNode, nameNode);
  }
  return idNode;
}
