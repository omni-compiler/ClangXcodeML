#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "LibXMLUtil.h"

/*!
 * \brief Search for an element that matches given XPath expression.
 * \return The first element that matches \c xpathExpr.
 */
xmlNodePtr findFirst(xmlNodePtr node, const char* xpathExpr, xmlXPathContextPtr xpathCtxt) {
  if (!xpathCtxt) {
    return nullptr;
  }
  xmlXPathSetContextNode(node, xpathCtxt);
  xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(
      node,
      BAD_CAST xpathExpr,
      xpathCtxt
      );
  if (!xpathObj || !(xpathObj->nodesetval)) {
    return nullptr;
  }
  return xpathObj->nodesetval->nodeTab[0];
}

