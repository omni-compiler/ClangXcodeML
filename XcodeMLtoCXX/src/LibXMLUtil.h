#ifndef LIBXMLUTIL_H
#define LIBXMLUTIL_H

xmlNodePtr findFirst(xmlNodePtr node, const char* xpathExpr, xmlXPathContextPtr xpathCtxt);
size_t length(xmlXPathObjectPtr obj);
xmlNodePtr nth(xmlXPathObjectPtr obj, size_t n);

#endif /* !LIBXMLUTIL_H */
