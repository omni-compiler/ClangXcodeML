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
  return src.typeTable[dataTypeIdent];
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
#define CB_ARGS const CodeBuilder& w __attribute__((unused)), \
                xmlNodePtr node __attribute__((unused)), \
                SourceInfo& src __attribute__((unused)), \
                std::stringstream& ss __attribute__((unused))
/*!
 * \brief Define new CodeBuilder::Procedure named \c name.
 */
#define DEFINE_CB(name) void name(CB_ARGS)

DEFINE_CB(EmptyProc) {
  w.walkChildren(node, src, ss);
}

CodeBuilder::Procedure outputStringLn(std::string str) {
  return [str](CB_ARGS) {
    ss << str << std::endl;
  };
}

CodeBuilder::Procedure outputString(std::string str) {
  return [str](CB_ARGS) {
    ss << str;
  };
}

CodeBuilder::Procedure handleBrackets(
    std::string opening,
    std::string closing,
    CodeBuilder::Procedure mainProc
) {
  return merge(outputString(opening),
               merge(mainProc, outputString(closing)));
}

CodeBuilder::Procedure handleBracketsLn(
    std::string opening,
    std::string closing,
    CodeBuilder::Procedure mainProc
) {
  return merge(outputStringLn(opening),
               merge(mainProc,outputStringLn(closing)));
}

/*!
 * \brief Make a procedure that handles binary operation.
 * \param Operator Spelling of binary operator.
 */
CodeBuilder::Procedure showBinOp(std::string Operator) {
  return [Operator](CB_ARGS) {
    xmlNodePtr lhs = findFirst(node, "*[1]", src.ctxt),
               rhs = findFirst(node, "*[2]", src.ctxt);
    ss << "(";
    w.walk(lhs, src, ss);
    ss << Operator;
    w.walk(rhs, src, ss);
    ss << ")";
  };
}

const CodeBuilder::Procedure EmptySNCProc = [](CB_ARGS) {
  ss << xmlNodeGetContent(node);
};

/*!
 * \brief Make a procedure that outputs text content of a given
 * XML element.
 * \param prefix Text to output before text content.
 * \param suffix Text to output after text content.
 */
CodeBuilder::Procedure showNodeContent(
    std::string prefix,
    std::string suffix
) {
  return handleBrackets(prefix, suffix, EmptySNCProc);
}

/*!
 * \brief Make a procedure that processes the first child element of
 * a given XML element (At least one element should exist in it).
 * \param prefix Text to output before traversing descendant elements.
 * \param suffix Text to output after traversing descendant elements.
 */
CodeBuilder::Procedure showChildElem(
    std::string prefix,
    std::string suffix
) {
  return handleBrackets(prefix, suffix, EmptyProc);
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

const CodeBuilder::Procedure handleScope =
  handleSymTableStack(handleBracketsLn("{", "}", EmptyProc));

DEFINE_CB(functionDefinitionProc) {
  xmlNodePtr nameElem = findFirst(
      node,
      "name|operator|constructor|destructor",
      src.ctxt
  );
  XMLString name(xmlNodeGetContent(nameElem));
  auto fnTypeName = findSymbolType(src.symTable, name);
  ss << TypeRefToString(src.typeTable.getReturnType(fnTypeName))
    << " "
    << name
    << "(";
  bool alreadyPrinted = false;
  for (auto p : src.symTable.back()) {
    if (alreadyPrinted) {
      ss << ", ";
    }
    auto paramType(getIdentType(src, p.first));
    ss << makeDecl(paramType, p.first);
    alreadyPrinted = true;
  }
  ss << ")" << std::endl;
  w.walkChildren(node, src, ss);
}

DEFINE_CB(memberRefProc) {
  w.walkChildren(node, src, ss);
  ss << "." << xmlGetProp(node, BAD_CAST "member");
}

DEFINE_CB(memberAddrProc) {
  ss << "&";
  memberRefProc(w, node, src, ss);
}

DEFINE_CB(memberPointerRefProc) {
  w.walkChildren(node, src, ss);
  ss << ".*" << xmlGetProp(node, BAD_CAST "name");
}

DEFINE_CB(compoundValueProc) {
  ss << "{";
  w.walkChildren(node, src, ss);
  ss << "}";
}

DEFINE_CB(thisExprProc) {
  ss << "this";
}

const auto compoundStatementProc = handleScope;

DEFINE_CB(whileStatementProc) {
  auto cond = findFirst(node, "condition", src.ctxt),
       body = findFirst(node, "body", src.ctxt);
  ss << "while (";
  w.walk(cond, src, ss);
  ss << ")" << std::endl;
  handleScope(w, body, src, ss);
}

DEFINE_CB(doStatementProc) {
  auto cond = findFirst(node, "condition", src.ctxt),
       body = findFirst(node, "body", src.ctxt);
  ss << "do ";
  handleScope(w, body, src, ss);
  ss << "while (";
  w.walk(cond, src, ss);
  ss  << ");" << std::endl;
}

DEFINE_CB(forStatementProc) {
  auto init = findFirst(node, "init", src.ctxt),
       cond = findFirst(node, "condition", src.ctxt),
       iter = findFirst(node, "iter", src.ctxt),
       body = findFirst(node, "body", src.ctxt);
  ss << "for (";
  if (init) {
    w.walk(init, src, ss);
  }
  ss << ";";
  if (cond) {
    w.walk(cond, src, ss);
  }
  ss << ";";
  if (iter) {
    w.walk(iter, src, ss);
  }
  ss << ")" << std::endl;
  handleScope(w, body, src, ss);
}

DEFINE_CB(returnStatementProc) {
  xmlNodePtr child = xmlFirstElementChild(node);
  if (child) {
    ss << "return ";
    w.walkAll(child, src, ss);
    ss << ";" << std::endl;
  } else {
    ss << "return;" << std::endl;
  }
}

DEFINE_CB(functionCallProc) {
  xmlNodePtr function = findFirst(node, "function/*", src.ctxt);
  w.walk(function, src, ss);
  xmlNodePtr arguments = findFirst(node, "arguments", src.ctxt);
  w.walk(arguments, src, ss);
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
    w.walk(arg, src, ss);
    alreadyPrinted = true;
  }
  ss << ")";
}

DEFINE_CB(condExprProc) {
  xmlNodePtr prd = findFirst(node, "*[1]", src.ctxt),
             the = findFirst(node, "*[2]", src.ctxt),
             els = findFirst(node, "*[3]", src.ctxt);
  w.walk(prd, src, ss);
  ss << " ? ";
  w.walk(the, src, ss);
  ss << " : ";
  w.walk(els, src, ss);
}

DEFINE_CB(varDeclProc) {
  xmlNodePtr nameElem = findFirst(node, "name", src.ctxt);
  XMLString name(xmlNodeGetContent(nameElem));
  auto type = getIdentType(src, name);
  ss << makeDecl(type, name);
  xmlNodePtr valueElem = findFirst(node, "value", src.ctxt);
  if (valueElem) {
    ss << " = ";
    w.walk(valueElem, src, ss);
  }
  ss << ";" << std::endl;
}

const CodeBuilder CXXBuilder({
  { "functionDefinition", handleSymTableStack(functionDefinitionProc) },
  { "intConstant", EmptySNCProc },
  { "moeConstant", EmptySNCProc },
  { "booleanConstant", EmptySNCProc },
  { "funcAddr", EmptySNCProc },
  { "stringConstant", showNodeContent("\"", "\"") },
  { "Var", EmptySNCProc },
  { "varAddr", showNodeContent("&", "") },
  { "pointerRef", showNodeContent("*", "") },
  { "memberRef", memberRefProc },
  { "memberAddr", memberAddrProc },
  { "memberPointerRef", memberPointerRefProc },
  { "compoundValue", compoundValueProc },
  { "compoundStatement", compoundStatementProc },
  { "whileStatement", whileStatementProc },
  { "doStatement", doStatementProc },
  { "forStatement", forStatementProc },
  { "thisExpr", thisExprProc },
  { "assignExpr", showBinOp(" = ") },
  { "plusExpr", showBinOp(" + ") },
  { "minusExpr", showBinOp(" - ") },
  { "mulExpr", showBinOp(" * ") },
  { "divExpr", showBinOp(" / ") },
  { "modExpr", showBinOp(" % ") },
  { "unaryMinusExpr", showUnaryOp("-") },
  { "binNotExpr", showUnaryOp("~") },
  { "logNotExpr", showUnaryOp("!") },
  { "sizeOfExpr", showUnaryOp("sizeof") },
  { "functionCall", functionCallProc },
  { "arguments", argumentsProc },
  { "condExpr", condExprProc },
  { "exprStatement", showChildElem("", ";\n") },
  { "returnStatement", returnStatementProc },
  { "varDecl", varDeclProc },
});

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
