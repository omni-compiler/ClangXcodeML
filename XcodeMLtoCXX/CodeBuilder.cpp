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

SymbolEntry parseSymbols(xmlNodePtr node, xmlXPathContextPtr ctxt) {
  xmlXPathObjectPtr xpathObj = xmlXPathNodeEval(node, BAD_CAST "id", ctxt);
  if (xpathObj == nullptr) {
    return SymbolEntry();
  }
  const size_t len = (xpathObj->nodesetval)? xpathObj->nodesetval->nodeNr:0;
  SymbolEntry entry;
  for (size_t i = 0; i < len; ++i) {
    xmlNodePtr idElem = xpathObj->nodesetval->nodeTab[i];
    xmlNodePtr nameElem = findFirst(idElem, "name", ctxt);
    XMLString type(xmlGetProp(idElem, BAD_CAST "type"));
    XMLString name(xmlNodeGetContent(nameElem));
    entry[name] = type;
  }
  return entry;
}

/* table shoud contain name */
std::string findSymbolType(const SymbolMap& table, const std::string& name) {
  for (auto entry : table) {
    auto result(entry.find(name));
    if (result != entry.end()) {
      return result->second;
    }
  }
  assert(false); /* due to constraint of parameters */
}

/* src.symTable should contain name */
XcodeMl::TypeRef getIdentType(const SourceInfo& src, const std::string& ident) {
  std::string dataTypeIdent(findSymbolType(src.symTable, ident));
  return src.typeTable.find(dataTypeIdent)->second;
}

SymbolMap parseGlobalSymbols(xmlDocPtr doc) {
  if (doc == nullptr) {
    return SymbolMap();
  }
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);
  if (xpathCtx == nullptr) {
    return SymbolMap();
  }
  xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
      BAD_CAST "/XcodeProgram/globalSymbols",
      xpathCtx);
  if (xpathObj == nullptr) {
    xmlXPathFreeContext(xpathCtx);
    return SymbolMap();
  }
  assert(xpathObj->nodesetval->nodeTab[0]);
  auto initialEntry(parseSymbols(xpathObj->nodesetval->nodeTab[0], xpathCtx));
  return {initialEntry};
}

#define CB_ARGS xmlNodePtr node __attribute__((unused)), \
                const CodeBuilder& r __attribute__((unused)), \
                SourceInfo& src __attribute__((unused)), \
                std::stringstream& ss __attribute__((unused))
#define DEFINE_CB(name) void name(CB_ARGS)

CodeBuilder::Procedure showBinOp(std::string operand) {
  return [operand](CB_ARGS) {
    xmlNodePtr lhs = findFirst(node, "*[1]", src.ctxt),
               rhs = findFirst(node, "*[2]", src.ctxt);
    ss << "(";
    r.callOnce(lhs, src, ss);
    ss << operand;
    r.callOnce(rhs, src, ss);
    ss << ")";
  };
}

CodeBuilder::Procedure showNodeContent(std::string prefix, std::string suffix) {
  return [prefix, suffix](CB_ARGS) {
    ss << prefix << xmlNodeGetContent(node) << suffix;
  };
}

CodeBuilder::Procedure EmptySNCProc = showNodeContent("", "");

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

CodeBuilder::Procedure showUnaryOp(std::string operand) {
  return showChildElem(operand + "(", ")");
}

CodeBuilder::Procedure handleSymTableStack(
    const CodeBuilder::Procedure mainProc) {
  const CodeBuilder::Procedure push = [](CB_ARGS) {
    SymbolEntry entry = parseSymbols(findFirst(node, "symbols", src.ctxt), src.ctxt);
    src.symTable.push_back(entry);
  };
  const CodeBuilder::Procedure pop = [](CB_ARGS) {
    src.symTable.pop_back();
  };
  return merge(push, merge(mainProc, pop));
}

DEFINE_CB(functionDefinitionProc) {
  xmlNodePtr nameElem = findFirst(
      node,
      "name|operator|constructor|destructor",
      src.ctxt
  );
  XMLString name(xmlNodeGetContent(nameElem));
  auto type(getFunctionType(getIdentType(src, name)));

  XMLString kind(nameElem->name);
  if (kind == "name" || kind == "operator") {
    ss << TypeRefToString(type.returnType)
      << " " << name;
  } else if (kind == "constructor") {
    ss << "<constructor>";
  } else if (kind == "destructor") {
    ss << "<destructor>";
  } else {
    assert(false);
  }

  ss << "(";
  bool alreadyPrinted = false;
  for (auto p : src.symTable.back()) {
    if (alreadyPrinted) {
      ss << ", ";
    }
    auto paramType(getIdentType(src, p.first));
    ss << TypeRefToString(paramType) << " " << p.first;
    alreadyPrinted = true;
  }
  ss << ")" << std::endl;

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
  xmlNodePtr nameElem = findFirst(node, "name", src.ctxt),
             valueElem = findFirst(node, "value", src.ctxt);
  XMLString name(xmlNodeGetContent(nameElem));
  auto type = getIdentType(src, name);
  ss << TypeRefToString(type) << " " << name << " = ";
  r.callOnce(valueElem, src, ss);
  ss << ";\n";
}

const CodeBuilder CXXBuilder = {
  std::make_tuple("functionDefinition", handleSymTableStack(functionDefinitionProc)),
  std::make_tuple("intConstant", EmptySNCProc),
  std::make_tuple("moeConstant", EmptySNCProc),
  std::make_tuple("booleanConstant", EmptySNCProc),
  std::make_tuple("funcAddr", EmptySNCProc),
  std::make_tuple("stringConstant", showNodeContent("\"", "\"")),
  std::make_tuple("Var", EmptySNCProc),
  std::make_tuple("varAddr", showNodeContent("&", "")),
  std::make_tuple("pointerRef", showNodeContent("*", "")),
  std::make_tuple("memberRef", memberRefProc),
  std::make_tuple("memberAddr", memberAddrProc),
  std::make_tuple("memberPointerRef", memberPointerRefProc),
  std::make_tuple("compoundValue", compoundValueProc),
  std::make_tuple("compoundStatement", handleSymTableStack(compoundStatementProc)),
  std::make_tuple("thisExpr", thisExprProc),
  std::make_tuple("assignExpr", showBinOp(" = ")),
  std::make_tuple("plusExpr", showBinOp(" + ")),
  std::make_tuple("minusExpr", showBinOp(" - ")),
  std::make_tuple("mulExpr", showBinOp(" * ")),
  std::make_tuple("divExpr", showBinOp(" / ")),
  std::make_tuple("modExpr", showBinOp(" % ")),
  std::make_tuple("unaryMinusExpr", showUnaryOp("-")),
  std::make_tuple("binNotExpr", showUnaryOp("~")),
  std::make_tuple("logNotExpr", showUnaryOp("!")),
  std::make_tuple("sizeOfExpr", showUnaryOp("sizeof")),
  std::make_tuple("functionCall", functionCallProc),
  std::make_tuple("arguments", argumentsProc),
  std::make_tuple("condExpr", condExprProc),
  std::make_tuple("exprStatement", showChildElem("", ";\n")),
  std::make_tuple("returnStatement", showChildElem("return ", ";\n")),
  std::make_tuple("varDecl", varDeclProc),
};

void buildCode(xmlDocPtr doc, std::stringstream& ss) {
  SourceInfo src = {
    xmlXPathNewContext(doc),
    parseTypeTable(doc),
    parseGlobalSymbols(doc)
  };
  CXXBuilder.call(xmlDocGetRootElement(doc), src, ss);
}
