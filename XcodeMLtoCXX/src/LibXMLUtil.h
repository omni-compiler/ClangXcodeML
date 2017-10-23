#ifndef LIBXMLUTIL_H
#define LIBXMLUTIL_H

struct XPathObjectReleaser {
  void operator()(xmlXPathObjectPtr ptr);
};

using XPathObjectSPtr = std::unique_ptr<xmlXPathObject, XPathObjectReleaser>;

xmlNodePtr findFirst(
    xmlNodePtr node, const char *xpathExpr, xmlXPathContextPtr xpathCtxt);
std::vector<xmlNodePtr> findNodes(
    xmlNodePtr node, const char *xpathExpr, xmlXPathContextPtr xpathCtxt);
size_t length(xmlXPathObjectPtr obj);
xmlNodePtr nth(xmlXPathObjectPtr obj, size_t n);
std::string getProp(xmlNodePtr node, const std::string &attr);
llvm::Optional<std::string> getPropOrNull(xmlNodePtr, const std::string &);
std::string getContent(xmlNodePtr);
std::string getName(xmlNodePtr);

/* Utility for XcodeML */
bool isTrueProp(xmlNodePtr node, const char *name, bool default_value);
bool isNaturalNumber(const std::string &);

#endif /* !LIBXMLUTIL_H */
