#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <sstream>

using TypeMap = std::map<std::string, std::string>;

class XMLString {
public:
  XMLString(const xmlChar*);
  XMLString(const char*);
  const xmlChar* c_ptr() const;
  operator std::string() {
    std::stringstream ss;
    ss << c_ptr();
    return ss.str();
  }
private:
  const xmlChar* ptr;
};

XMLString::XMLString(const xmlChar * p) : ptr(p) {}

XMLString::XMLString(const char *str) : ptr(BAD_CAST str) {}

const xmlChar* XMLString::c_ptr() const {
  return ptr;
}

XMLString operator+(const XMLString lhs, const XMLString rhs) {
  xmlChar* dst = xmlStrdup(lhs.c_ptr());
  return xmlStrcat(dst, rhs.c_ptr());
}

bool operator==(const XMLString lhs, const XMLString rhs) {
  return xmlStrEqual(lhs.c_ptr(), rhs.c_ptr());
}

size_t length(XMLString str) {
  return static_cast<size_t>(xmlStrlen(str.c_ptr()));
}

TypeMap parseTypeTable(xmlDocPtr doc);

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
    return 0;
  }
  std::string filename(argv[1]);
  xmlDocPtr doc = xmlParseFile(filename.c_str());
  TypeMap t = parseTypeTable(doc);
  return 0;
}

XMLString xmlNodePtrToTypeName(xmlNodePtr node) {
  XMLString type = node->name;
  if (type == "typeName" || type == "basicType") {
    return xmlGetProp(node, BAD_CAST "ref");
  } else if (type == "pointerType") {
    XMLString type = xmlStrdup(xmlGetProp(node, BAD_CAST "ref"));
    return type + "*";
  }
  return "Type";
}

xmlNodePtr find(xmlNodePtr node, const char* xpathExpr) {

}

void buildCode(xmlNodePtr node, std::stringstream ss) {
  XMLString nodeType = node->name;
  if (nodeType == "functionDefinition") {
    xmlNodePtr 
  }
}

TypeMap parseTypeTable(xmlDocPtr doc) {
  if (doc == nullptr) {
    return TypeMap();
  }
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
  if (xpathCtx == nullptr) {
    return TypeMap();
  }
  xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
      BAD_CAST "/XcodeProgram/typeTable/*",
      xpathCtx);
  if (xpathObj == nullptr) {
    xmlXPathFreeContext(xpathCtx);
    return TypeMap();
  }
  const size_t len = (xpathObj->nodesetval)? xpathObj->nodesetval->nodeNr:0;
  TypeMap val;
  for (int i = 0; i < len; ++i) {
    xmlNodePtr cur = xpathObj->nodesetval->nodeTab[i];
    std::stringstream ss;
    ss << cur->name;
    val[ss.str()] = xmlNodePtrToTypeName(cur);
  }
  return TypeMap();
}

