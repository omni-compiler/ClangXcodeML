#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <sstream>
#include <cassert>
#include "XMLString.h"

using TypeMap = std::map<std::string, std::string>;

TypeMap parseTypeTable(xmlDocPtr doc);
void buildCode(xmlNodePtr, std::stringstream&, xmlXPathContextPtr);

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
    return 0;
  }
  std::string filename(argv[1]);
  xmlDocPtr doc = xmlParseFile(filename.c_str());
  TypeMap t = parseTypeTable(doc);
  std::stringstream ss;
  xmlXPathContextPtr xpathCtxt = xmlXPathNewContext(doc);
  buildCode(xmlDocGetRootElement(doc), ss, xpathCtxt);
  std::cout << ss.str() << std::endl;
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

std::function<void()> flushLineFn(std::stringstream& ss, std::string text) {
  return [&ss, text](){
    ss << text;
  };
}

void buildCode(xmlNodePtr node, std::stringstream& ss, xmlXPathContextPtr xpathCtxt) {
  std::function<void()> postprocess = [](){ };
  bool stop_traversing = false;
  for (xmlNodePtr cur = node; cur; cur = cur->next) {
    if (cur->type == XML_ELEMENT_NODE) {
      XMLString elemName = cur->name;
      if (elemName == "functionDefinition") {
        xmlNodePtr fnName = findFirst(
            cur,
            "name|operator|constructor|destructor",
            xpathCtxt);
        XMLString fnType = fnName->name;
        if (fnType == "name" || fnType == "operator") {
          ss << xmlNodeGetContent(fnName);
        } else if (fnType == "constructor") {
          ss << "<constructor>";
        } else if (fnType == "destructor") {
          ss << "<destructor>";
        } else {
          assert(false);
        }
        ss << "()\n";
      } else if (elemName == "compoundStatement") {
        ss << "{ /* compoundStatement */\n";
        postprocess = flushLineFn(ss, "} /*compoundStatement */\n");
      } else if (elemName == "symbols") {
        ss << "/* symbols */\n";
        //stop_traversing = true;
      }
    }
    if (!stop_traversing) {
      buildCode(cur->children, ss, xpathCtxt);
    }
  }
  postprocess();
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

