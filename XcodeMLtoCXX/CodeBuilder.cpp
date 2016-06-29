#include <functional>
#include <sstream>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "XMLString.h"
#include "XMLWalker.h"
#include "XcodeMlType.h"
#include "TypeAnalyzer.h"
#include "CodeBuilder.h"
#include "LibXMLUtil.h"

/*!
 * \brief Traverse XcodeML node and make SymbolEntry.
 * \pre \c node is <globalSymbols> or <symbols> element.
 */
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

/*!
 * \brief Search \c table for \c name visible .
 * \pre \c table contains \c name.
 * \return Data type identifier of \c name.
 */
std::string findSymbolType(const SymbolMap& table, const std::string& name) {
  for (auto entry : table) {
    auto result(entry.find(name));
    if (result != entry.end()) {
      return result->second;
    }
  }
  assert(false); /* due to constraint of parameters */
}

/*!
 * \brief Search for \c ident visible in current scope.
 * \pre src.symTable contains \c ident.
 * \return Data type of \c ident.
 */
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

/*!
 * \brief Arguments to be passed to CodeBuilder::Procedure.
 */
#define CB_ARGS xmlNodePtr node __attribute__((unused)), \
                const CodeBuilder& r __attribute__((unused)), \
                SourceInfo& src __attribute__((unused)), \
                std::stringstream& ss __attribute__((unused))
/*!
 * \brief Define new CodeBuilder::Procedure named \c name.
 */
#define DEFINE_CB(name) void name(CB_ARGS)

/*!
 * \brief Make a procedure that handles binary operation.
 * \param Operator Spelling of binary operator.
 */
CodeBuilder::Procedure showBinOp(std::string Operator) {
  return [Operator](CB_ARGS) {
    xmlNodePtr lhs = findFirst(node, "*[1]", src.ctxt),
               rhs = findFirst(node, "*[2]", src.ctxt);
    ss << "(";
    r.walk(lhs, src, ss);
    ss << Operator;
    r.walk(rhs, src, ss);
    ss << ")";
  };
}

/*!
 * \brief Make a procedure that outputs text content of a given
 * XML element.
 * \param prefix Text to output before text content.
 * \param suffix Text to output after text content.
 */
CodeBuilder::Procedure showNodeContent(std::string prefix, std::string suffix) {
  return [prefix, suffix](CB_ARGS) {
    ss << prefix << xmlNodeGetContent(node) << suffix;
  };
}

CodeBuilder::Procedure EmptySNCProc = showNodeContent("", "");

/*!
 * \brief Make a procedure that processes the first child element of
 * a given XML element (At least one element should exist in it).
 * \param prefix Text to output before traversing descendant elements.
 * \param suffix Text to output after traversing descendant elements.
 */
CodeBuilder::Procedure showChildElem(std::string prefix, std::string suffix) {
  return [prefix, suffix](CB_ARGS) {
    ss << prefix;
    xmlNodePtr target = node->children;
    while (target->type != XML_ELEMENT_NODE) {
      target = target->next;
    }
    r.walk(target, src, ss);
    ss << suffix;
  };
}

/*!
 * \brief Make a procedure that handles unary operation.
 * \param Operator Spelling of unary operator.
 */
CodeBuilder::Procedure showUnaryOp(std::string Operator) {
  return showChildElem(Operator + "(", ")");
}

/*!
 * \brief Make a procedure that handles SourceInfo::symTable.
 * \return A procudure. It traverses <symbols> node and pushes new
 * entry to symTable before running \c mainProc. After \c mainProc,
 * it pops the entry.
 */
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

  r.walkAll(node->children, src, ss);
}

DEFINE_CB(memberRefProc) {
  r.walkAll(node->children, src, ss);
  ss << "." << xmlGetProp(node, BAD_CAST "member");
}

DEFINE_CB(memberAddrProc) {
  ss << "&";
  memberRefProc(node, r, src, ss);
}

DEFINE_CB(memberPointerRefProc) {
  r.walkAll(node->children, src, ss);
  ss << ".*" << xmlGetProp(node, BAD_CAST "name");
}

DEFINE_CB(compoundValueProc) {
  ss << "{";
  r.walkAll(node->children, src, ss);
  ss << "}";
}

DEFINE_CB(thisExprProc) {
  ss << "this";
}

DEFINE_CB(compoundStatementProc) {
  ss << "{\n";
  r.walkAll(node->children, src, ss);
  ss << "}\n";
}

DEFINE_CB(functionCallProc) {
  xmlNodePtr function = findFirst(node, "function/*", src.ctxt);
  r.walk(function, src, ss);
  xmlNodePtr arguments = findFirst(node, "arguments", src.ctxt);
  r.walk(arguments, src, ss);
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
    r.walk(arg, src, ss);
    alreadyPrinted = true;
  }
  ss << ")";
}

DEFINE_CB(condExprProc) {
  xmlNodePtr prd = findFirst(node, "*[1]", src.ctxt),
             the = findFirst(node, "*[2]", src.ctxt),
             els = findFirst(node, "*[3]", src.ctxt);
  r.walk(prd, src, ss);
  ss << " ? ";
  r.walk(the, src, ss);
  ss << " : ";
  r.walk(els, src, ss);
}

DEFINE_CB(varDeclProc) {
  xmlNodePtr nameElem = findFirst(node, "name", src.ctxt),
             valueElem = findFirst(node, "value", src.ctxt);
  XMLString name(xmlNodeGetContent(nameElem));
  auto type = getIdentType(src, name);
  ss << TypeRefToString(type) << " " << name << " = ";
  r.walk(valueElem, src, ss);
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

/*!
 * \brief Traverse an XcodeML document and generate C++ source code.
 * \param[in] doc XcodeML document.
 * \param[out] ss Stringstream to flush C++ source code.
 */
void buildCode(xmlDocPtr doc, std::stringstream& ss) {
  SourceInfo src = {
    xmlXPathNewContext(doc),
    parseTypeTable(doc),
    parseGlobalSymbols(doc)
  };
  CXXBuilder.walkAll(xmlDocGetRootElement(doc), src, ss);
}
