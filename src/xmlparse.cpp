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
#include "Reality.h"

using TypeMap = std::map<std::string, std::string>;

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

#define NP_ARGS xmlNodePtr node, SourceInfo src, std::stringstream& ss, const Reality& r
#define DEFINE_NP(name) void name(NP_ARGS)

DEFINE_NP(functionDefinitionProc) {
  xmlNodePtr fnName = findFirst(node, "name|operator|constructor|destructor", src.ctxt);
  XMLString fnType = fnName->name;
  std::cout << static_cast<std::string>( fnType ) << std::endl;
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

DEFINE_NP(memberRefProc) {
  r.call(node->children, src, ss);
  ss << "." << xmlGetProp(node, BAD_CAST "member");
}

DEFINE_NP(memberAddrProc) {
  ss << "&";
  memberRefProc(node, src, ss, r);
}

DEFINE_NP(memberPointerRefProc) {
  r.call(node->children, src, ss);
  ss << ".*" << xmlGetProp(node, BAD_CAST "name");
}

DEFINE_NP(compoundValueProc) {
  ss << "{";
  r.call(node->children, src, ss);
  ss << "}";
}

DEFINE_NP(thisExprProc) {
  ss << "this";
}

DEFINE_NP(compoundStatementProc) {
  ss << "{\n";
  r.call(node->children, src, ss);
  ss << "}\n";
}

DEFINE_NP(functionCallProc) {
  xmlNodePtr function = findFirst(node, "function/*", src.ctxt);
  r.callOnce(function, src, ss);
  xmlNodePtr arguments = findFirst(node, "arguments", src.ctxt);
  r.callOnce(arguments, src, ss);
}

DEFINE_NP(argumentsProc) {
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

DEFINE_NP(condExprProc) {
  xmlNodePtr prd = findFirst(node, "*[1]", src.ctxt),
             the = findFirst(node, "*[2]", src.ctxt),
             els = findFirst(node, "*[3]", src.ctxt);
  r.callOnce(prd, src, ss);
  ss << " ? ";
  r.callOnce(the, src, ss);
  ss << " : ";
  r.callOnce(els, src, ss);
}

DEFINE_NP(varDeclProc) {
  xmlNodePtr name = findFirst(node, "name", src.ctxt),
             value = findFirst(node, "value", src.ctxt);
  ss << xmlNodeGetContent(name) << " = ";
  r.callOnce(value, src, ss);
  ss << ";\n";
}

NodeProcessor showBinOp(std::string operand) {
  return [operand](NP_ARGS) {
    xmlNodePtr lhs = findFirst(node, "*[1]", src.ctxt),
               rhs = findFirst(node, "*[2]", src.ctxt);
    r.callOnce(lhs, src, ss);
    ss << operand;
    r.callOnce(rhs, src, ss);
  };
}

NodeProcessor showNodeContent(std::string prefix, std::string suffix) {
  return [prefix, suffix](NP_ARGS) {
    ss << prefix << xmlNodeGetContent(node) << suffix;
  };
}

NodeProcessor showChildElem(std::string prefix, std::string suffix) {
  return [prefix, suffix](NP_ARGS) {
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
  Reality r;
  const NodeProcessor snc = showNodeContent("", "");
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

