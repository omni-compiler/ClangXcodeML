#include <map>
#include <memory>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "LibXMLUtil.h"
#include "StringTree.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"
#include "XcodeMlOperator.h"
#include "XMLString.h"

#include "XcodeMlUtil.h"

std::shared_ptr<XcodeMl::UnqualId>
getNameFromNameNode(xmlNodePtr nameNode) {
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
getNameFromIdNode(xmlNodePtr idNode, xmlXPathContextPtr ctxt) {
  if (!idNode) {
    throw std::domain_error("expected id node, but got null");
  }
  xmlNodePtr nameNode = findFirst(idNode, "name", ctxt);
  if (!nameNode) {
    throw std::domain_error("name node not found");
  }
  return getNameFromNameNode(nameNode);
}
