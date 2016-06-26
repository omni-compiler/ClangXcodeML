#include <functional>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <memory>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "XMLString.h"
#include "Reality.h"
#include "XcodeMlType.h"
#include "TypeAnalyzer.h"

#define TA_ARGS xmlNodePtr node, const TypeAnalyzer& r, TypeMap& map
#define DEFINE_TA(name) void name(TA_ARGS)

DEFINE_TA(basicTypeProc) {
  XMLString protoName = xmlGetProp(node, BAD_CAST "name");
  auto prototype = map[protoName];
  XMLString name(xmlGetProp(node, BAD_CAST "type"));
  map[name] = prototype;
}

DEFINE_TA(pointerTypeProc) {
  XMLString refName = xmlGetProp(node, BAD_CAST "ref");
  auto ref = map[refName];
  XMLString name(xmlGetProp(node, BAD_CAST "type"));
  map[name] = makePointerType(ref);
}

DEFINE_TA(functionTypeProc) {
  XMLString returnName = xmlGetProp(node, BAD_CAST "return_type");
  auto returnType = map[returnName];
  XMLString name(xmlGetProp(node, BAD_CAST "type"));
  map[name] = makeFunctionType(returnType, {});
}

DEFINE_TA(arrayTypeProc) {
  XMLString elemName = xmlGetProp(node, BAD_CAST "element_type");
  auto elemType = map[elemName];
  XMLString name(xmlGetProp(node, BAD_CAST "type"));
  map[name] = makeArrayType(elemType, 0);
}

const std::vector<std::string> dataTypeIdents = {
  "void",
  "char",
  "short",
  "int",
  "long",
  "long_long",
  "unsigned_char",
  "unsigned_short",
  "unsigned",
  "unsigned_long",
  "unsigned_long_long",
  "float",
  "double",
  "long_double",
  "wchar_t",
  "char16_t",
  "char32_t",
  "bool",
};


XcodeMlTypeRef makeReservedType(std::string);

const TypeMap dataTypeIdentMap = [](const std::vector<std::string>& keys) {
  TypeMap map;
  for (std::string key : keys) {
    map[key] = makeReservedType(key);
  }
  return map;
}(dataTypeIdents);

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
  TypeAnalyzer ta;
  TypeMap map(dataTypeIdentMap);
  ta.registerNP("basicType", basicTypeProc);
  ta.registerNP("pointerType", pointerTypeProc);
  ta.registerNP("functionType", functionTypeProc);
  ta.registerNP("arrayType", arrayTypeProc);
  for (size_t i = 0; i < len; ++i) {
    xmlNodePtr node = xpathObj->nodesetval->nodeTab[i];
    ta.callOnce(node, map);
  }
  return map;
}

