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
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "Symbol.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SymbolAnalyzer.h"
#include "TypeAnalyzer.h"

using TypeAnalyzer = XMLWalker<xmlXPathContextPtr, XcodeMl::Environment&>;

/*!
 * \brief Arguments to be passed to TypeAnalyzer::Procedure.
 */
#define TA_ARGS const TypeAnalyzer& w __attribute__((unused)), \
                xmlNodePtr node __attribute__((unused)), \
                xmlXPathContextPtr ctxt __attribute__((unused)), \
                XcodeMl::Environment& map __attribute__((unused))
/*!
 * \brief Define new TypeAnalyzer::Procedure named \c name.
 */
#define DEFINE_TA(name) void name(TA_ARGS)

DEFINE_TA(basicTypeProc) {
  XMLString signified = xmlGetProp(node, BAD_CAST "name");
  XMLString signifier(xmlGetProp(node, BAD_CAST "type"));
  map[signifier] = XcodeMl::makeReservedType(
      signifier,
      signified,
      isTrueProp(node, "is_const", false),
      isTrueProp(node, "is_volatile", false)
  );
}

DEFINE_TA(pointerTypeProc) {
  XMLString refName = xmlGetProp(node, BAD_CAST "ref");
  XMLString name(xmlGetProp(node, BAD_CAST "type"));
  auto pointer = XcodeMl::makePointerType(name, refName);
  pointer->setConst(isTrueProp(node, "is_const", false));
  pointer->setVolatile(isTrueProp(node, "is_volatile", false));
  map[name] = pointer;
}

DEFINE_TA(functionTypeProc) {
  XMLString returnName = xmlGetProp(node, BAD_CAST "return_type");
  auto returnType = map[returnName];
  xmlXPathObjectPtr paramsNode = xmlXPathNodeEval(
      node,
      BAD_CAST "params/*",
      ctxt
  );
  XcodeMl::Function::Params params;
  for (size_t i = 0, len = length(paramsNode); i < len; ++i) {
    xmlNodePtr param = nth(paramsNode, i);
    XMLString paramType(xmlGetProp(param, BAD_CAST "type"));
    XMLString paramName(xmlNodeGetContent(param));
    params.emplace_back(paramType, paramName);
  }
  XMLString name(xmlGetProp(node, BAD_CAST "type"));
  map.setReturnType(name, returnType);
  map[name] = XcodeMl::makeFunctionType(name, returnType, params);
}

DEFINE_TA(arrayTypeProc) {
  using XcodeMl::Array;

  XMLString elemName = xmlGetProp(node, BAD_CAST "element_type");
  auto elemType = map[elemName];
  XMLString name(xmlGetProp(node, BAD_CAST "type"));

  const Array::Size size = [node]() {
    if (!xmlHasProp(node, BAD_CAST "array_size")) {
      return Array::Size::makeVariableSize();
    }
    const XMLString size_prop = xmlGetProp(node, BAD_CAST "array_size");
    return size_prop == "*" ?
      Array::Size::makeVariableSize() :
      Array::Size::makeIntegerSize(std::stoi(size_prop));
  }();

  auto array = XcodeMl::makeArrayType(name, elemType, size);
  array->setConst(isTrueProp(node, "is_const", false));
  array->setVolatile(isTrueProp(node, "is_volatile", false));
  map[name] = array;
}

static XcodeMl::Struct::Member makeMember(xmlNodePtr idNode) {
  XMLString type = xmlGetProp(idNode, BAD_CAST "type");
  XMLString name = xmlNodeGetContent(xmlFirstElementChild(idNode));
  return XcodeMl::Struct::Member(type, name);
}

DEFINE_TA(structTypeProc) {
  XMLString elemName = xmlGetProp(node, BAD_CAST "type");
  std::vector<XcodeMl::Struct::Member> fields;
  const auto symbols = findNodes(node, "symbols/id", ctxt);
  for (auto& symbol : symbols) {
    fields.push_back(makeMember(symbol));
  }
  map[elemName] = XcodeMl::makeStructType(elemName, "", fields);
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
const XcodeMl::Environment dataTypeIdentMap = [](const std::vector<std::string>& keys) {
  XcodeMl::Environment map;
  for (std::string key : keys) {
    map[key] = XcodeMl::makeReservedType(key, key);
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
XcodeMl::Environment parseTypeTable(xmlDocPtr doc) {
  if (doc == nullptr) {
    return XcodeMl::Environment();
  }
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
  if (xpathCtx == nullptr) {
    return XcodeMl::Environment();
  }
  xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
      BAD_CAST "/XcodeProgram/typeTable/*",
      xpathCtx);
  if (xpathObj == nullptr) {
    xmlXPathFreeContext(xpathCtx);
    return XcodeMl::Environment();
  }
  const size_t len = length(xpathObj);
  XcodeMl::Environment map(dataTypeIdentMap);
  for (size_t i = 0; i < len; ++i) {
    xmlNodePtr node = nth(xpathObj, i);
    XcodeMLTypeAnalyzer.walk(node, xpathCtx, map);
  }
  xmlXPathFreeObject(xpathObj);
  xmlNodePtr globalSymbols = findFirst(
      xmlDocGetRootElement(doc),
      "/XcodeProgram/globalSymbols",
      xpathCtx);
  analyzeSymbols(globalSymbols, xpathCtx, map);
  return map;
}

