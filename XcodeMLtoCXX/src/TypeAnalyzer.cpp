#include <algorithm>
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
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "StringTree.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"
#include "XcodeMlType.h"
#include "XcodeMlUtil.h"
#include "XcodeMlTypeTable.h"
#include "TypeAnalyzer.h"

using TypeAnalyzer =
    XMLWalker<void, xmlXPathContextPtr, XcodeMl::TypeTable &>;

using CXXCodeGen::makeTokenNode;
using CXXCodeGen::makeVoidNode;

/*!
 * \brief Arguments to be passed to TypeAnalyzer::Procedure.
 */
#define TA_ARGS                                                               \
  const TypeAnalyzer &w __attribute__((unused)),                              \
      xmlNodePtr node __attribute__((unused)),                                \
      xmlXPathContextPtr ctxt __attribute__((unused)),                        \
      XcodeMl::TypeTable &map __attribute__((unused))
/*!
 * \brief Define new TypeAnalyzer::Procedure named \c name.
 */
#define DEFINE_TA(name) static void name(TA_ARGS)

DEFINE_TA(basicTypeProc) {
  XMLString signified = xmlGetProp(node, BAD_CAST "name");
  XMLString signifier(xmlGetProp(node, BAD_CAST "type"));
  map[signifier] = XcodeMl::makeQualifiedType(signifier,
      signified,
      isTrueProp(node, "is_const", false),
      isTrueProp(node, "is_volatile", false));
}

DEFINE_TA(pointerTypeProc) {
  XMLString refName = xmlGetProp(node, BAD_CAST "ref");
  XMLString name(xmlGetProp(node, BAD_CAST "type"));

  const auto refProp = getPropOrNull(node, "reference");
  if (refProp.hasValue() && (*refProp == "lvalue")) {
    auto reference = XcodeMl::makeLValueReferenceType(name, refName);
    map[name] = reference;
    return;
  }

  auto pointer = XcodeMl::makePointerType(name, refName);
  pointer->setConst(isTrueProp(node, "is_const", false));
  pointer->setVolatile(isTrueProp(node, "is_volatile", false));
  map[name] = pointer;
}

DEFINE_TA(functionTypeProc) {
  const auto returnDTI = getProp(node, "return_type");
  const auto returnType = map[returnDTI];
  xmlXPathObjectPtr paramsNode =
      xmlXPathNodeEval(node, BAD_CAST "params/paramTypeName", ctxt);
  std::vector<XcodeMl::DataTypeIdent> paramTypes;
  for (size_t i = 0, len = length(paramsNode); i < len; ++i) {
    xmlNodePtr ithParamNode = nth(paramsNode, i);
    const auto paramType = getProp(ithParamNode, "type");
    paramTypes.push_back(paramType);
  }
  const auto dtident = getProp(node, "type");
  map.setReturnType(dtident, returnType);
  const auto func = findFirst(node, "params/ellipsis", ctxt)
      ? XcodeMl::makeVariadicFunctionType(dtident, returnDTI, paramTypes)
      : XcodeMl::makeFunctionType(dtident, returnDTI, paramTypes);
  func->setConst(isTrueProp(node, "is_const", false));
  func->setVolatile(isTrueProp(node, "is_volatile", false));
  map[dtident] = func;
}

DEFINE_TA(arrayTypeProc) {
  using XcodeMl::Array;

  XMLString elemName = xmlGetProp(node, BAD_CAST "element_type");
  XMLString name(xmlGetProp(node, BAD_CAST "type"));

  const Array::Size size = [node]() {
    if (!xmlHasProp(node, BAD_CAST "array_size")) {
      return Array::Size::makeVariableSize();
    }
    const XMLString size_prop = xmlGetProp(node, BAD_CAST "array_size");
    return size_prop == "*"
        ? Array::Size::makeVariableSize()
        : Array::Size::makeIntegerSize(std::stoi(size_prop));
  }();

  auto array = XcodeMl::makeArrayType(name, elemName, size);
  array->setConst(isTrueProp(node, "is_const", false));
  array->setVolatile(isTrueProp(node, "is_volatile", false));
  map[name] = array;
}

static XcodeMl::MemberDecl
makeMember(xmlNodePtr idNode) {
  XMLString type = xmlGetProp(idNode, BAD_CAST "type");
  XMLString name = xmlNodeGetContent(xmlFirstElementChild(idNode));
  if (!xmlHasProp(idNode, BAD_CAST "bit_field")) {
    return XcodeMl::MemberDecl(type, makeTokenNode(name));
  }
  XMLString bit_size = xmlGetProp(idNode, BAD_CAST "bit_field");
  if (!isNaturalNumber(bit_size)) {
    return XcodeMl::MemberDecl(type, makeTokenNode(name));
    // FIXME: Don't ignore <bitField> element
  }
  return XcodeMl::MemberDecl(type, makeTokenNode(name), std::stoi(bit_size));
}

DEFINE_TA(structTypeProc) {
  XMLString elemName = xmlGetProp(node, BAD_CAST "type");
  XcodeMl::Struct::MemberList fields;
  const auto symbols = findNodes(node, "symbols/id", ctxt);
  for (auto &symbol : symbols) {
    fields.push_back(makeMember(symbol));
  }
  map[elemName] = XcodeMl::makeStructType(elemName, makeVoidNode(), fields);
}

static std::vector<XcodeMl::ClassType::BaseClass>
getBases(xmlNodePtr node, xmlXPathContextPtr ctxt) {
  auto nodes = findNodes(node, "inheritedFrom/typeName", ctxt);
  std::vector<XcodeMl::ClassType::BaseClass> result;
  std::transform(nodes.begin(),
      nodes.end(),
      std::back_inserter(result),
      [](xmlNodePtr node) {
        return std::make_tuple(getProp(node, "access"),
            getProp(node, "ref"),
            isTrueProp(node, "is_virtual", 0));
      });
  return result;
}

llvm::Optional<XcodeMl::ClassType::TemplateArgList>
getTemplateArgs(xmlNodePtr node, xmlXPathContextPtr ctxt) {
  using namespace XcodeMl;
  using MaybeList = llvm::Optional<ClassType::TemplateArgList>;
  const auto targsNode = findFirst(node, "templateArguments", ctxt);
  if (!targsNode) {
    return MaybeList();
  }
  ClassType::TemplateArgList targs;
  for (auto &&targNode : findNodes(targsNode, "typeName", ctxt)) {
    targs.push_back(getProp(targNode, "ref"));
  }
  return MaybeList(targs);
}

DEFINE_TA(classTypeProc) {
  XMLString elemName = xmlGetProp(node, BAD_CAST "type");
  const auto nameSpelling = getContent(findFirst(node, "name", ctxt));
  const auto className = nameSpelling.empty() ? XcodeMl::ClassType::ClassName()
                                              : makeTokenNode(nameSpelling);
  const auto bases = getBases(node, ctxt);
  const auto targs = getTemplateArgs(node, ctxt);
  XcodeMl::ClassType::Symbols symbols;
  const auto ids = findNodes(node, "symbols/id", ctxt);
  for (auto &idElem : ids) {
    const auto dtident = getProp(idElem, "type");
    const auto pName = getUnqualIdFromIdNode(idElem, ctxt);
    symbols.emplace_back(pName, dtident);
  }
  const auto nnsident = getPropOrNull(node, "nns");

  const auto classKind = getProp(node, "cxx_class_kind");
  if (classKind == "union") {
    map[elemName] =
        XcodeMl::makeCXXUnionType(elemName, className, bases, symbols, targs);
  } else {
    map[elemName] = XcodeMl::makeClassType(
        elemName, nnsident, className, bases, symbols, targs);
  }
}

DEFINE_TA(enumTypeProc) {
  const auto dtident = getType(node);
  const auto name = getUnqualIdFromIdNode(node, ctxt);
  map[dtident] = XcodeMl::makeEnumType(dtident, name);
}

DEFINE_TA(TemplateTypeParmTypeProc) {
  const auto dtident = getProp(node, "type");
  const auto name = getContent(findFirst(node, "name", ctxt));
  map[dtident] = XcodeMl::makeTemplateTypeParm(dtident, makeTokenNode(name));
}

const std::vector<std::string> identicalFndDataTypeIdents = {
    "void",
    "char",
    "short",
    "int",
    "long",
    "unsigned",
    "float",
    "double",
    "wchar_t",
    "char16_t",
    "char32_t",
    "bool",
    "__int128",
    "nullptr_t",
};

const std::vector<std::tuple<std::string, std::string>>
    nonidenticalFndDataTypeIdents = {
        std::make_tuple("long_long", "long long"),
        std::make_tuple("unsigned_char", "unsigned char"),
        std::make_tuple("unsigned_short", "unsigned short"),
        std::make_tuple("unsigned_int", "unsigned int"),
        // out of specification
        std::make_tuple("unsigned_long", "unsigned long"),
        std::make_tuple("unsigned_long_long", "unsigned long long"),
        std::make_tuple("long_double", "long double"),
        std::make_tuple("unsigned___int128", "unsigned __int128"),
        std::make_tuple("signed_char", "signed char"),
};

/*!
 * \brief Mapping from Data type identifiers to basic data types.
 */
const XcodeMl::TypeTable FundamentalDataTypeIdentMap = []() {
  XcodeMl::TypeTable map;
  for (std::string key : identicalFndDataTypeIdents) {
    map[key] = XcodeMl::makeReservedType(key, makeTokenNode(key));
  }
  for (auto p : nonidenticalFndDataTypeIdents) {
    std::string ident, type_name;
    std::tie(ident, type_name) = p;
    map[ident] = XcodeMl::makeReservedType(ident, makeTokenNode(type_name));
  }
  return map;
}();

const TypeAnalyzer XcodeMLTypeAnalyzer("TypeAnalyzer",
    {
        std::make_tuple("basicType", basicTypeProc),
        std::make_tuple("pointerType", pointerTypeProc),
        std::make_tuple("functionType", functionTypeProc),
        std::make_tuple("arrayType", arrayTypeProc),
        std::make_tuple("structType", structTypeProc),
        std::make_tuple("classType", classTypeProc),
        std::make_tuple("enumType", enumTypeProc),
        std::make_tuple("TemplateTypeParmType", TemplateTypeParmTypeProc),
        std::make_tuple("injectedClassNameType", classTypeProc),
    });

/*!
 * \brief Traverse an XcodeML document and make mapping from data
 * type identifiers to data types defined in it.
 */
XcodeMl::TypeTable
parseTypeTable(xmlNodePtr, xmlXPathContextPtr xpathCtx, std::stringstream &) {
  xmlXPathObjectPtr xpathObj =
      xmlXPathEvalExpression(BAD_CAST "/XcodeProgram/typeTable/*", xpathCtx);
  if (xpathObj == nullptr) {
    return XcodeMl::TypeTable();
  }
  const size_t len = length(xpathObj);
  XcodeMl::TypeTable map(FundamentalDataTypeIdentMap);
  for (size_t i = 0; i < len; ++i) {
    xmlNodePtr node = nth(xpathObj, i);
    XcodeMLTypeAnalyzer.walk(node, xpathCtx, map);
  }
  xmlXPathFreeObject(xpathObj);
  return map;
}

XcodeMl::TypeTable
expandTypeTable(const XcodeMl::TypeTable &env,
    xmlNodePtr typeTable,
    xmlXPathContextPtr ctxt) {
  auto newEnv = env;
  const auto definitions = findNodes(typeTable, "*", ctxt);
  for (auto &&definition : definitions) {
    XcodeMLTypeAnalyzer.walk(definition, ctxt, newEnv);
  }
  return newEnv;
}
