#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <libxml/debugXML.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "llvm/ADT/Optional.h"
#include "LibXMLUtil.h"
#include "StringTree.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"
#include "XcodeMlUtil.h"
#include "XMLString.h"

static xmlXPathObjectPtr getNodeSet(
    xmlNodePtr, const char *, xmlXPathContextPtr);

void XPathObjectReleaser::operator()(xmlXPathObjectPtr ptr) {
  xmlXPathFreeObject(ptr);
}

/*!
 * \brief Search for an element that matches given XPath expression.
 * \pre \c node is not null.
 * \pre \c xpathExpr is not null.
 * \pre \c xpathCtxt is not null.
 * \return nullptr if any element in \c node doesn't match \c xpathExpr
 * \return The first element that matches \c xpathExpr.
 */
xmlNodePtr
findFirst(
    xmlNodePtr node, const char *xpathExpr, xmlXPathContextPtr xpathCtxt) {
  assert(node);
  xmlXPathObjectPtr xpathObj = getNodeSet(node, xpathExpr, xpathCtxt);
  if (!xpathObj) {
    return nullptr;
  }
  xmlNodePtr val = xpathObj->nodesetval->nodeTab[0];
  xmlXPathFreeObject(xpathObj);
  return val;
}

size_t
length(xmlXPathObjectPtr obj) {
  return (obj->nodesetval) ? obj->nodesetval->nodeNr : 0;
}

xmlNodePtr
nth(xmlXPathObjectPtr obj, size_t n) {
  return obj->nodesetval->nodeTab[n];
}

std::string
getProp(xmlNodePtr node, const std::string &attr) {
  const auto value = getPropOrNull(node, attr);
  if (!value.hasValue()) {
    std::cerr << "getProp: " << attr << " not found" << std::endl;
    std::cerr << getXcodeMlPath(node) << std::endl;
    xmlDebugDumpNode(stderr, node, 0);
    std::abort();
  }
  return *value;
}

llvm::Optional<std::string>
getPropOrNull(xmlNodePtr node, const std::string &attr) {
  using MaybeString = llvm::Optional<std::string>;
  const auto ptr = xmlGetProp(node, BAD_CAST attr.c_str());
  if (!ptr) {
    return MaybeString();
  }
  const auto value = static_cast<XMLString>(ptr);
  xmlFree(ptr);
  return MaybeString(value);
}

std::string
getContent(xmlNodePtr node) {
  const auto ptr = xmlNodeGetContent(node);
  if (!ptr) {
    return "";
  }
  const auto content = static_cast<XMLString>(ptr);
  xmlFree(ptr);
  return content;
}

std::string
getName(xmlNodePtr node) {
  return static_cast<XMLString>(node->name);
}

bool
isTrueProp(xmlNodePtr node, const char *name, bool default_value) {
  if (!xmlHasProp(node, BAD_CAST name)) {
    return default_value;
  }
  const auto value = getProp(node, name);
  if (value == "1" || value == "true") {
    return true;
  } else if (value == "0" || value == "false") {
    return false;
  }
  throw std::runtime_error("Invalid attribute value");
}

std::vector<xmlNodePtr>
findNodes(
    xmlNodePtr node, const char *xpathExpr, xmlXPathContextPtr xpathCtxt) {
  xmlXPathObjectPtr xpathObj = getNodeSet(node, xpathExpr, xpathCtxt);
  if (!xpathObj) {
    return {};
  }
  std::vector<xmlNodePtr> nodes;
  xmlNodeSetPtr matchedNodes = xpathObj->nodesetval;
  const int len = matchedNodes->nodeNr;
  for (int i = 0; i < len; ++i) {
    nodes.push_back(matchedNodes->nodeTab[i]);
  }
  xmlXPathFreeObject(xpathObj);
  return nodes;
}

bool
isNaturalNumber(const std::string &prop) {
  return std::all_of(prop.begin(), prop.end(), isdigit);
}

static xmlXPathObjectPtr
getNodeSet(
    xmlNodePtr node, const char *xpathExpr, xmlXPathContextPtr xpathCtxt) {
  assert(node && xpathExpr);
  xmlXPathSetContextNode(node, xpathCtxt);
  xmlXPathObjectPtr xpathObj =
      xmlXPathNodeEval(node, BAD_CAST xpathExpr, xpathCtxt);
  if (!xpathObj) {
    return nullptr;
  }
  xmlNodeSetPtr matchedNodes = xpathObj->nodesetval;
  if (!matchedNodes || !matchedNodes->nodeNr) {
    xmlXPathFreeObject(xpathObj);
    return nullptr;
  }
  return xpathObj;
}
