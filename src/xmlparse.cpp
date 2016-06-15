#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <sstream>
#include "XMLString.h"

using TypeMap = std::map<std::string, std::string>;

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

