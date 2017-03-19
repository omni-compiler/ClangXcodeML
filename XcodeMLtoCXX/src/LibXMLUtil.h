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
std::string getProp(xmlNodePtr node, const std::string& attr);
llvm::Optional<std::string> getPropOrNull(xmlNodePtr, const std::string&);

/* Utility for XcodeML */
bool isTrueProp(xmlNodePtr node, const char* name, bool default_value);
std::string getNameFromIdNode(xmlNodePtr idNode, xmlXPathContextPtr ctxt);
llvm::Optional<std::string> getNameFromIdNodeOrNull(xmlNodePtr idNode, xmlXPathContextPtr ctxt);
bool isNaturalNumber(const std::string&);

#endif /* !LIBXMLUTIL_H */
