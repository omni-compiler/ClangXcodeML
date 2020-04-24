#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>
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
 * \brief Search for a single XML node that matches the given XPath expression.
 *
 * \param node      the XML context node of \c xpathExpr
 * \param xpathExpr the XPath expression
 * \param xpathCtxt the XPath context object
 *
 * \pre \c node is not null.
 * \pre \c xpathExpr is not null.
 * \pre \c xpathCtxt is not null.
 *
 * \return Returns null if none of the XML nodes in \c node matches
 * \c xpathExpr.
 * Otherwise, returns the first element that matches \c xpathExpr.
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

/*!
 * \brief Returns the number of XML nodes that belong to the XPath search
 * result.
 */
size_t
length(xmlXPathObjectPtr obj) {
  return (obj->nodesetval) ? obj->nodesetval->nodeNr : 0;
}

/*!
 * \brief Returns the \c n th XML node (0-indexed) of XPath search result.
 *
 * \pre the size (the number of nodes containing) of \c obj is greater than \c
 * n.
 */
xmlNodePtr
nth(xmlXPathObjectPtr obj, size_t n) {
  return obj->nodesetval->nodeTab[n];
}

/*!
 * \brief Returns the value of the attribute on the XML node
 * as \c std::string.
 *
 * This function is a wrapper of \c xmlGetProp.
 * It terminates the whole program if \c node does not have
 * the attribute \c attr.
 *
 * \pre \c node has the attribute named \c attr.
 */
std::string
getProp(xmlNodePtr node, const std::string &attr) {
  if (node == nullptr){
    std::cerr << attr<<std::endl;
    throw(std::runtime_error("Node is Null"));
  }
  const auto value = getPropOrNull(node, attr);
  if (!value.hasValue()) {
    std::cerr << "getProp: " << attr << " not found" << std::endl;
    std::cerr << getXcodeMlPath(node) << std::endl;
    xmlDebugDumpNode(stderr, node, 0);
    std::abort();
  }
  return *value;
}

/*!
 * \brief Returns the value of the attribute on the XML node
 * as \c std::string.
 *
 * This function is a wrapper of \c xmlGetProp.
 *
 * \return Returns \c llvm::Optional<std::string>() if
 * the specified attribute does not exist.
 * Otherwise, returns the value of the attribute.
 */
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

/*!
 * \brief Returns the XML content text.
 *
 * This function is a wrapper of \c xmlNodeGetContent.
 */
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

/*!
 * \brief Returns the XML element name as \c std::string.
 */
std::string
getName(xmlNodePtr node) {
  if(!node)
      throw(std::runtime_error("Node is null"));
  return static_cast<XMLString>(node->name);
}

/*!
 * \brief Determine if the specified XML node does not contain XML content
 * text.
 */
bool
isEmpty(xmlNodePtr node) {
  return getContent(node).empty();
}

/*!
 * \brief Determine if the value of the specified attribute of the XML node
 * evaluates to true.
 *
 * An attribute value evaluates to true if it is \c "true" or \c "1",
 * or false if it is \c "false" or "0".
 *
 * \pre The attribute value is one of:
 * - "true"
 * - "false"
 * - "1"
 * - "0"
 *
 * \param node the XML node
 * \param name the name of attribute
 * \param default_value the default value of the attribute (true or false).
 *
 * \throw std::runtime_error if the attribute value is invalid.
 */
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

/*!
 * \brief Search for XML nodes that matches the given XPath expression.
 *
 * \param node      the XML context node of \c xpathExpr
 * \param xpathExpr the XPath expression
 * \param xpathCtxt the XPath context object
 *
 * \return Returns the list of XML nodes that match \c xpathExpr.
 * If none of the XML nodes in \c node matches \c xpathExpr, it returns empty
 * list.
 */
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

/*!
 * \brief Determine if the given string is a natural number (including 0).
 */
bool
isNaturalNumber(const std::string &prop) {
  return std::all_of(prop.begin(), prop.end(), isdigit);
}

/*!
 * \brief Search for XML nodes that matches the given XPath expression.
 *
 * \param node      the XML context node of \c xpathExpr
 * \param xpathExpr the XPath expression
 * \param xpathCtxt the XPath context object
 *
 * \return Returns null if such node does not exist.
 * Otherwise, returns the XPath search result.
 * The caller has to free the search result object.
 */
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
