#ifndef LIBXMLUTIL_H
#define LIBXMLUTIL_H

struct XPathObjectReleaser {
  void operator() (xmlXPathObjectPtr ptr);
};

using XPathObjectSPtr = std::unique_ptr<xmlXPathObject, XPathObjectReleaser>;

xmlNodePtr findFirst(xmlNodePtr node, const char* xpathExpr, xmlXPathContextPtr xpathCtxt);
std::vector<xmlNodePtr> findNodes(xmlNodePtr node, const char* xpathExpr, xmlXPathContextPtr xpathCtxt);
size_t length(xmlXPathObjectPtr obj);
xmlNodePtr nth(xmlXPathObjectPtr obj, size_t n);

/* Utility for XcodeML */
bool isTrueProp(xmlNodePtr node, const char* name, bool default_value);
std::string getNameFromIdNode(xmlNodePtr idNode, xmlXPathContextPtr ctxt);

#endif /* !LIBXMLUTIL_H */
