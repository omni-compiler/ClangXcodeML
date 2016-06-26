#include <functional>
#include <sstream>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "XMLString.h"
#include "Reality.h"
#include "XcodeMlType.h"
#include "TypeAnalyzer.h"
#include "CodeBuilder.h"
#include "LibXMLUtil.h"

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
