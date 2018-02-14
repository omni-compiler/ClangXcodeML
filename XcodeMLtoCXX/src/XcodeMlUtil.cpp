#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <libxml/debugXML.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "StringTree.h"
#include "Util.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"
#include "XcodeMlOperator.h"
#include "XMLString.h"
#include "SourceInfo.h"

#include "XcodeMlUtil.h"

std::shared_ptr<XcodeMl::UnqualId>
getUnqualIdFromNameNode(xmlNodePtr nameNode) {
  const auto kind = getProp(nameNode, "name_kind");

  if (kind == "constructor") {
    const auto dtident = getProp(nameNode, "ctor_type");
    return std::make_shared<XcodeMl::CtorName>(dtident);
  } else if (kind == "destructor") {
    const auto dtident = getProp(nameNode, "dtor_type");
    return std::make_shared<XcodeMl::DtorName>(dtident);
  } else if (kind == "operator") {
    const auto opName = getContent(nameNode);
    const auto pOpId = XcodeMl::OperatorNameToSpelling(opName);
    assert(pOpId.hasValue());
    return std::make_shared<XcodeMl::OpFuncId>(*pOpId);
  } else if (kind == "conversion") {
    const auto dtident = getProp(nameNode, "destination_type");
    return std::make_shared<XcodeMl::ConvFuncId>(dtident);
  }

  assert(kind == "name");
  const auto name = getContent(nameNode);
  return std::make_shared<XcodeMl::UIDIdent>(name);
}

std::shared_ptr<XcodeMl::UnqualId>
getUnqualIdFromIdNode(xmlNodePtr idNode, xmlXPathContextPtr ctxt) {
  if (!idNode) {
    throw std::domain_error("expected id node, but got null");
  }
  xmlNodePtr nameNode = findFirst(idNode, "name", ctxt);
  if (!nameNode) {
    throw std::domain_error("name node not found");
  }
  return getUnqualIdFromNameNode(nameNode);
}

std::shared_ptr<XcodeMl::Nns>
getNns(const XcodeMl::NnsMap &nnsTable, xmlNodePtr nameNode) {
  const auto ident = getPropOrNull(nameNode, "nns");
  if (!ident.hasValue()) {
    return std::shared_ptr<XcodeMl::Nns>();
  }
  const auto nns = getOrNull(nnsTable, *ident);
  if (!nns.hasValue()) {
    const auto lineno = xmlGetLineNo(nameNode);
    assert(lineno >= 0);
    std::cerr << "Undefined NNS: '" << *ident << "'" << std::endl
              << "lineno: " << lineno << std::endl;
    xmlDebugDumpNode(stderr, nameNode, 0);
    std::abort();
  }
  return *nns;
}

XcodeMl::Name
getQualifiedNameFromNameNode(xmlNodePtr nameNode, const SourceInfo &src) {
  const auto id = getUnqualIdFromNameNode(nameNode);
  if (src.language == Language::C) {
    return XcodeMl::Name(id);
  }
  const auto nns = getNns(src.nnsTable, nameNode);
  return XcodeMl::Name(id, nns);
}

void
xcodeMlPwd(xmlNodePtr node, std::ostream &os) {
  assert(node);
  if (!(node->parent)) {
    os << "/";
    return;
  }

  xcodeMlPwd(node->parent, os);
  const auto name = getName(node);
  const auto comment = static_cast<std::string>("(:")
      + std::to_string(xmlGetLineNo(node)) + static_cast<std::string>(":)");

  const std::map<std::string, std::string> specialNodes = {
      {"clangStmt", "class"}, {"clangDecl", "class"},
  };
  const auto iter = specialNodes.find(name);
  if (iter == specialNodes.end()) {
    os << name << comment << "/";
    return;
  }
  const auto attrVal = getPropOrNull(node, iter->second);
  if (!attrVal.hasValue()) {
    os << name << comment << "(:no @" << iter->second << ":)/";
    return;
  }
  os << name << "[@" << iter->second << "='" << *attrVal << "']" << comment
     << "/";
}

XcodeMlPwdType
getXcodeMlPath(xmlNodePtr node) {
  return {node};
}

std::vector<XcodeMl::CodeFragment>
getParamNames(xmlNodePtr fnNode, const SourceInfo &src) {
  std::vector<XcodeMl::CodeFragment> vec;
  const auto params = findNodes(
      fnNode, "clangTypeLoc/clangDecl[@class='ParmVar']/name", src.ctxt);
  for (auto p : params) {
    XMLString name = xmlNodeGetContent(p);
    vec.push_back(CXXCodeGen::makeTokenNode(name));
  }
  return vec;
}

XcodeMl::CodeFragment
makeFunctionDeclHead(XcodeMl::Function *func,
    const XcodeMl::Name &name,
    const std::vector<XcodeMl::CodeFragment> &paramNames,
    const SourceInfo &src,
    bool emitNameSpec) {
  const auto pUnqualId = name.getUnqualId();
  const auto nameSpelling = emitNameSpec
      ? name.toString(src.typeTable, src.nnsTable)
      : pUnqualId->toString(src.typeTable);
  if (llvm::isa<XcodeMl::CtorName>(pUnqualId.get())
      || llvm::isa<XcodeMl::DtorName>(pUnqualId.get())
      || llvm::isa<XcodeMl::ConvFuncId>(pUnqualId.get())) {
    /* Do not emit return type
     *    void A::A();
     *    void A::~A();
     *    int A::operator int();
     */
    return func->makeDeclarationWithoutReturnType(
        nameSpelling, paramNames, src.typeTable);
  } else {
    return func->makeDeclaration(nameSpelling, paramNames, src.typeTable);
  }
}

std::string
getType(xmlNodePtr node) {
  const auto type = getPropOrNull(node, "xcodemlType");
  if (type.hasValue()) {
    return *type;
  }

  return getProp(node, "type");
}

XcodeMl::CodeFragment
makeFunctionDeclHead(xmlNodePtr node,
    const std::vector<XcodeMl::CodeFragment> paramNames,
    const SourceInfo &src,
    bool emitNameSpec) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto name = getQualifiedNameFromNameNode(nameNode, src);

  const auto dtident = getType(node);
  const auto T = src.typeTable[dtident];
  const auto fnType = llvm::cast<XcodeMl::Function>(T.get());

  auto acc = CXXCodeGen::makeVoidNode();
  acc = acc + makeFunctionDeclHead(fnType,
                  name,
                  paramNames,
                  src,
                  emitNameSpec && xmlHasProp(node, BAD_CAST "parent_class"));
  return acc;
}

XcodeMl::CodeFragment
wrapWithLangLink(const XcodeMl::CodeFragment &content,
    xmlNodePtr node,
    const SourceInfo &src) {
  using namespace CXXCodeGen;
  if (src.language != Language::CPlusPlus) {
    return content;
  }
  const auto lang = getPropOrNull(node, "language_linkage");
  if (!lang.hasValue() || *lang == "C++") {
    return content;
  } else {
    return makeTokenNode("extern") + makeTokenNode("\"" + *lang + "\"")
        + makeTokenNode("{") + content + makeTokenNode("}");
  }
}

std::ostream &operator<<(std::ostream &os, const XcodeMlPwdType &x) {
  xcodeMlPwd(x.node, os);
  return os;
}
