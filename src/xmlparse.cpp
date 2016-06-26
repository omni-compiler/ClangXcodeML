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
#include "XcodeMlType.h"
#include "Reality.h"

TypeMap parseTypeTable(xmlDocPtr doc);
void buildCode(xmlDocPtr, std::stringstream&);

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
  buildCode(doc, ss);
  std::cout << ss.str() << std::endl;
  return 0;
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

using CodeBuilder = Reality<const SourceInfo&, std::stringstream&>;
#define CB_ARGS xmlNodePtr node, const CodeBuilder& r, const SourceInfo& src, std::stringstream& ss
#define DEFINE_CB(name) void name(CB_ARGS)

DEFINE_CB(functionDefinitionProc) {
  xmlNodePtr fnName = findFirst(node, "name|operator|constructor|destructor", src.ctxt);
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
  r.call(node->children, src, ss);
}

DEFINE_CB(memberRefProc) {
  r.call(node->children, src, ss);
  ss << "." << xmlGetProp(node, BAD_CAST "member");
}

DEFINE_CB(memberAddrProc) {
  ss << "&";
  memberRefProc(node, r, src, ss);
}

DEFINE_CB(memberPointerRefProc) {
  r.call(node->children, src, ss);
  ss << ".*" << xmlGetProp(node, BAD_CAST "name");
}

DEFINE_CB(compoundValueProc) {
  ss << "{";
  r.call(node->children, src, ss);
  ss << "}";
}

DEFINE_CB(thisExprProc) {
  ss << "this";
}

DEFINE_CB(compoundStatementProc) {
  ss << "{\n";
  r.call(node->children, src, ss);
  ss << "}\n";
}

DEFINE_CB(functionCallProc) {
  xmlNodePtr function = findFirst(node, "function/*", src.ctxt);
  r.callOnce(function, src, ss);
  xmlNodePtr arguments = findFirst(node, "arguments", src.ctxt);
  r.callOnce(arguments, src, ss);
}

DEFINE_CB(argumentsProc) {
  ss << "(";
  bool alreadyPrinted = false;
  for (xmlNodePtr arg = node->children; arg; arg = arg->next) {
    if (arg->type != XML_ELEMENT_NODE) {
      continue;
    }
    if (alreadyPrinted) {
      ss << ",";
    }
    r.callOnce(arg, src, ss);
    alreadyPrinted = true;
  }
  ss << ")";
}

DEFINE_CB(condExprProc) {
  xmlNodePtr prd = findFirst(node, "*[1]", src.ctxt),
             the = findFirst(node, "*[2]", src.ctxt),
             els = findFirst(node, "*[3]", src.ctxt);
  r.callOnce(prd, src, ss);
  ss << " ? ";
  r.callOnce(the, src, ss);
  ss << " : ";
  r.callOnce(els, src, ss);
}

DEFINE_CB(varDeclProc) {
  xmlNodePtr name = findFirst(node, "name", src.ctxt),
             value = findFirst(node, "value", src.ctxt);
  ss << xmlNodeGetContent(name) << " = ";
  r.callOnce(value, src, ss);
  ss << ";\n";
}

CodeBuilder::Procedure showBinOp(std::string operand) {
  return [operand](CB_ARGS) {
    xmlNodePtr lhs = findFirst(node, "*[1]", src.ctxt),
               rhs = findFirst(node, "*[2]", src.ctxt);
    r.callOnce(lhs, src, ss);
    ss << operand;
    r.callOnce(rhs, src, ss);
  };
}

CodeBuilder::Procedure showNodeContent(std::string prefix, std::string suffix) {
  return [prefix, suffix](CB_ARGS) {
    ss << prefix << xmlNodeGetContent(node) << suffix;
  };
}

CodeBuilder::Procedure showChildElem(std::string prefix, std::string suffix) {
  return [prefix, suffix](CB_ARGS) {
    ss << prefix;
    xmlNodePtr target = node->children;
    while (target->type != XML_ELEMENT_NODE) {
      target = target->next;
    }
    r.callOnce(target, src, ss);
    ss << suffix;
  };
}

void buildCode(xmlDocPtr doc, std::stringstream& ss) {
  CodeBuilder r;
  const CodeBuilder::Procedure snc = showNodeContent("", "");
  r.registerNP("functionDefinition", functionDefinitionProc);
  r.registerNP("intConstant", snc);
  r.registerNP("moeConstant", snc);
  r.registerNP("booleanConstant", snc);
  r.registerNP("funcAddr", snc);
  r.registerNP("stringConstant", showNodeContent("\"", "\""));
  r.registerNP("Var", snc);
  r.registerNP("varAddr", showNodeContent("&", ""));
  r.registerNP("pointerRef", showNodeContent("*", ""));
  r.registerNP("memberRef", memberRefProc);
  r.registerNP("memberAddr", memberAddrProc);
  r.registerNP("memberPointerRef", memberPointerRefProc);
  r.registerNP("compoundValue", compoundValueProc);
  r.registerNP("compoundStatement", compoundStatementProc);
  r.registerNP("thisExpr", thisExprProc);
  r.registerNP("assignExpr", showBinOp(" = "));
  r.registerNP("plusExpr", showBinOp(" + "));
  r.registerNP("minusExpr", showBinOp(" - "));
  r.registerNP("mulExpr", showBinOp(" * "));
  r.registerNP("divExpr", showBinOp(" / "));
  r.registerNP("modExpr", showBinOp(" % "));
  r.registerNP("unaryMinusExpr", showChildElem("-", ""));
  r.registerNP("binNotExpr", showChildElem("~", ""));
  r.registerNP("logNotExpr", showChildElem("!", ""));
  r.registerNP("sizeOfExpr", showChildElem("sizeof ", ""));
  r.registerNP("functionCall", functionCallProc);
  r.registerNP("arguments", argumentsProc);
  r.registerNP("condExpr", condExprProc);
  r.registerNP("exprStatement", showChildElem("", ";\n"));
  r.registerNP("returnStatement", showChildElem("return ", ";\n"));
  r.registerNP("varDecl", varDeclProc);

  SourceInfo src = {xmlXPathNewContext(doc), parseTypeTable(doc)};
  r.call(xmlDocGetRootElement(doc), src, ss);
}

using TypeAnalyzer = Reality<TypeMap&>;
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

