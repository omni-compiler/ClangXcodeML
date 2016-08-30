#include <cassert>
#include <string>
#include <sstream>
#include <stdexcept>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "LibXMLUtil.h"
#include "XMLString.h"

/*!
 * \brief Search for an element that matches given XPath expression.
 * \pre \c node is not null.
 * \pre \c xpathExpr is not null.
 * \pre \c xpathCtxt is not null.
 * \return nullptr if any element in \c node doesn't match \c xpathExpr
 * \return The first element that matches \c xpathExpr.
 */
xmlNodePtr findFirst(xmlNodePtr node, const char* xpathExpr, xmlXPathContextPtr xpathCtxt) {
  assert(node && xpathExpr);
  xmlXPathSetContextNode(node, xpathCtxt);
  xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(
      node,
      BAD_CAST xpathExpr,
      xpathCtxt
      );
  if (!xpathObj) {
    return nullptr;
  }
  xmlNodeSetPtr matchedNodes = xpathObj->nodesetval;
  if (!matchedNodes || !matchedNodes->nodeNr) {
    xmlXPathFreeObject(xpathObj);
    return nullptr;
  }
  return matchedNodes->nodeTab[0];
}

size_t length(xmlXPathObjectPtr obj) {
  return (obj->nodesetval)? obj->nodesetval->nodeNr:0;
}

xmlNodePtr nth(xmlXPathObjectPtr obj, size_t n) {
  return obj->nodesetval->nodeTab[n];
}

bool isTrueProp(xmlNodePtr node, const char* name, bool default_value) {
  if (!xmlHasProp(node, BAD_CAST name)) {
    return default_value;
  }
  std::string value = static_cast<XMLString>(xmlGetProp(node, BAD_CAST name));
  if (value == "1" || value == "true") {
    return true;
  } else if (value == "0" || value == "false") {
    return false;
  }
  throw std::runtime_error("Invalid attribute value");
}
