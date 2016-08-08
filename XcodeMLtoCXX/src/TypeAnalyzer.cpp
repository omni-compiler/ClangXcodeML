#include <functional>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <memory>
#include <cassert>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include "LibXMLUtil.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "SymbolAnalyzer.h"
#include "XcodeMlType.h"
#include "TypeAnalyzer.h"

using TypeAnalyzer = XMLWalker<TypeMap&>;

const XcodeMl::TypeRef& TypeMap::operator[](
  const std::string& dataTypeIdent
) const {
  return map.at(dataTypeIdent);
}

XcodeMl::TypeRef& TypeMap::operator[](
  const std::string& dataTypeIdent
) {
  if (map.find(dataTypeIdent) == map.end()) {
    keys.push_back(dataTypeIdent);
  }
  return map[dataTypeIdent];
}

const XcodeMl::TypeRef& TypeMap::getReturnType(
  const std::string& dataTypeIdent
) const {
  return returnMap.at(dataTypeIdent);
}

void TypeMap::setReturnType(
  const std::string& dataTypeIdent,
  const XcodeMl::TypeRef& type
) {
  returnMap[dataTypeIdent] = type;
}

const std::vector<std::string>& TypeMap::getKeys(void) const {
  return keys;
}

/*!
 * \brief Arguments to be passed to TypeAnalyzer::Procedure.
 */
#define TA_ARGS const TypeAnalyzer& w __attribute__((unused)), \
                xmlNodePtr node __attribute__((unused)), \
                TypeMap& map __attribute__((unused))
/*!
 * \brief Define new TypeAnalyzer::Procedure named \c name.
 */
#define DEFINE_TA(name) void name(TA_ARGS)

DEFINE_TA(basicTypeProc) {
  XMLString signified = xmlGetProp(node, BAD_CAST "name");
  auto signifiedType = map[signified];
  XMLString signifier(xmlGetProp(node, BAD_CAST "type"));
  map[signifier] = signifiedType;
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
  map.setReturnType(name, returnType);
  map[name] = makeFunctionType(returnType, {});
}

DEFINE_TA(arrayTypeProc) {
  XMLString elemName = xmlGetProp(node, BAD_CAST "element_type");
  auto elemType = map[elemName];
  XMLString name(xmlGetProp(node, BAD_CAST "type"));
  map[name] = makeArrayType(elemType, 0);
}

DEFINE_TA(structTypeProc) {
  // under construction
  XMLString elemName = xmlGetProp(node, BAD_CAST "type");
  //xmlNodePtr symTab = findFirst(node, "../" xpathCtx)
  SymbolMap fields;

  map[elemName] = XcodeMl::makeStructType(elemName, "", std::move(fields));
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

/*!
 * \brief Mapping from Data type identifiers to basic data types.
 */
const TypeMap dataTypeIdentMap = [](const std::vector<std::string>& keys) {
  TypeMap map;
  for (std::string key : keys) {
    map[key] = XcodeMl::makeReservedType(key);
  }
  return map;
}(dataTypeIdents);

const TypeAnalyzer XcodeMLTypeAnalyzer({
  { "basicType", basicTypeProc },
  { "pointerType", pointerTypeProc },
  { "functionType", functionTypeProc },
  { "arrayType", arrayTypeProc },
  { "structType", structTypeProc },
});

/*!
 * \brief Traverse an XcodeML document and make mapping from data
 * type identifiers to data types defined in it.
 */
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
  const size_t len = length(xpathObj);
  TypeMap map(dataTypeIdentMap);
  for (size_t i = 0; i < len; ++i) {
    xmlNodePtr node = nth(xpathObj, i);
    XcodeMLTypeAnalyzer.walk(node, map);
  }
  return map;
}

