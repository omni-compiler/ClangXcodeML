#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <libxml/debugXML.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
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

std::ostream &operator<<(std::ostream &os, const XcodeMlPwdType &x) {
  xcodeMlPwd(x.node, os);
  return os;
}
