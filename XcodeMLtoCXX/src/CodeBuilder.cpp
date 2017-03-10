#include <functional>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/Support/Casting.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "AttrProc.h"
#include "Stream.h"
#include "StringTree.h"
#include "Symbol.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "TypeAnalyzer.h"
#include "SourceInfo.h"
#include "CodeBuilder.h"
#include "ClangClassHandler.h"
#include "SymbolBuilder.h"
#include "LibXMLUtil.h"

namespace cxxgen = CXXCodeGen;

using cxxgen::StringTreeRef;
using cxxgen::makeTokenNode;
using cxxgen::makeInnerNode;
using cxxgen::makeNewLineNode;
using cxxgen::makeVoidNode;

using cxxgen::separateByBlankLines;

/*!
 * \brief Traverse XcodeML node and make SymbolEntry.
 * \pre \c node is <globalSymbols> or <symbols> element.
 */
SymbolEntry parseSymbols(xmlNodePtr node, xmlXPathContextPtr ctxt) {
  if (!node) {
    return SymbolEntry();
  }
  SymbolEntry entry;
  auto idElems = findNodes(node, "id", ctxt);
  for (auto idElem : idElems) {
    xmlNodePtr nameElem = findFirst(idElem, "name", ctxt);
    const auto type = getProp(idElem, "type");
    assert(type.length() != 0);
    XMLString name(xmlNodeGetContent(nameElem));
    if (!static_cast<std::string>(name).empty()) {
      // Ignore unnamed parameters such as <name type="int"/>
      entry[name] = type;
    }
  }
  return entry;
}

/*!
 * \brief Search \c table for \c name visible .
 * \pre \c table contains \c name.
 * \return Data type identifier of \c name.
 */
std::string findSymbolType(const SymbolMap& table, const std::string& name) {
  for (auto iter = table.rbegin(); iter != table.rend(); ++iter) {
    auto entry(*iter);
    auto result(entry.find(name));
    if (result != entry.end()) {
      return result->second;
    }
  }
  std::stringstream log;
  log << std::endl << "{";
  for (auto entry : table) {
    log << "\t{";
    for (auto p : entry) {
      log << "(" << p.first << "," << p.second << ")";
    }
    log << "}" << std::endl;
  }
  log << "}" << std::endl;
  throw std::runtime_error(
      name + " not found in SymbolMap: " + log.str());
}

/*!
 * \brief Search for \c ident visible in current scope.
 * \pre src.symTable contains \c ident.
 * \return Data type of \c ident.
 */
XcodeMl::TypeRef getIdentType(const SourceInfo& src, const std::string& ident) {
  assert(!ident.empty());
  std::string dataTypeIdent(findSymbolType(src.symTable, ident));
  return src.typeTable[dataTypeIdent];
}

SymbolMap parseGlobalSymbols(
    xmlNodePtr,
    xmlXPathContextPtr xpathCtx,
    std::stringstream&
) {
  xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(
      BAD_CAST "/XcodeProgram/globalSymbols",
      xpathCtx);
  if (xpathObj == nullptr
      || !xmlXPathCastNodeSetToBoolean(xpathObj->nodesetval)) {
    std::cerr
      << "Warning: This document does not have globalSymbols element"
      << std::endl;
    return SymbolMap();
  }
  assert(xpathObj
      && xpathObj->nodesetval
      && xpathObj->nodesetval->nodeTab
      && xpathObj->nodesetval->nodeTab[0]);
  auto initialEntry(parseSymbols(xpathObj->nodesetval->nodeTab[0], xpathCtx));
  return {initialEntry};
}

/*!
 * \brief Arguments to be passed to CodeBuilder::Procedure.
 */
#define CB_ARGS const CodeBuilder& w __attribute__((unused)), \
                xmlNodePtr node __attribute__((unused)), \
                SourceInfo& src __attribute__((unused))

/*!
 * \brief Define new CodeBuilder::Procedure named \c name.
 */
#define DEFINE_CB(name) static StringTreeRef name(CB_ARGS)

DEFINE_CB(NullProc) {
  return CXXCodeGen::makeVoidNode();
}

DEFINE_CB(EmptyProc) {
  return makeInnerNode( w.walkChildren(node, src) );
}

DEFINE_CB(walkChildrenWithInsertingNewLines) {
  return insertNewLines( w.walkChildren(node, src) );
}

CodeBuilder::Procedure outputStringLn(std::string str) {
  return [str](CB_ARGS) {
    return makeTokenNode(str) + makeNewLineNode();
  };
}

CodeBuilder::Procedure outputString(std::string str) {
  return [str](CB_ARGS) {
    return makeTokenNode(str);
  };
}

CodeBuilder::Procedure handleBrackets(
    std::string opening,
    std::string closing,
    CodeBuilder::Procedure mainProc
) {
  return [opening, closing, mainProc](CB_ARGS) {
    return makeTokenNode(opening) +
      mainProc(w, node, src) +
      makeTokenNode(closing);
  };
}

CodeBuilder::Procedure handleBracketsLn(
    std::string opening,
    std::string closing,
    CodeBuilder::Procedure mainProc
) {
  return [opening, closing, mainProc](CB_ARGS) {
    return makeTokenNode(opening) +
      mainProc(w, node, src) +
      makeTokenNode(closing) +
      makeNewLineNode();
  };
}

/*!
 * \brief Make a procedure that handles binary operation.
 * \param Operator Spelling of binary operator.
 */
CodeBuilder::Procedure showBinOp(std::string Operator) {
  return [Operator](CB_ARGS) {
    xmlNodePtr lhs = findFirst(node, "*[1]", src.ctxt),
               rhs = findFirst(node, "*[2]", src.ctxt);
    return
      makeTokenNode( "(" ) +
      w.walk(lhs, src) +
      makeTokenNode(Operator) +
      w.walk(rhs, src) +
      makeTokenNode(")");
  };
}

const CodeBuilder::Procedure EmptySNCProc = [](CB_ARGS) {
  return makeTokenNode(XMLString(xmlNodeGetContent(node)));
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
  return [mainProc](CB_ARGS) {
    SymbolEntry entry = parseSymbols(findFirst(node, "symbols", src.ctxt), src.ctxt);
    src.symTable.push_back(entry);
    auto ret = mainProc(w, node, src);
    src.symTable.pop_back();
    return ret;
  };
}

CodeBuilder::Procedure handleIndentation(
  const CodeBuilder::Procedure mainProc
) {
  return [mainProc](CB_ARGS) {
    return mainProc(w, node, src);
  };
}

const CodeBuilder::Procedure handleScope =
  handleBracketsLn("{", "}",
  handleIndentation(
  handleSymTableStack(
  walkChildrenWithInsertingNewLines)));

std::vector<XcodeMl::CodeFragment>
getParams(xmlNodePtr fnNode, const SourceInfo& src) {
  std::vector<XcodeMl::CodeFragment> vec;
  const auto params = findNodes(fnNode, "params/name", src.ctxt);
  for (auto p : params) {
    XMLString name = xmlNodeGetContent(p);
    vec.push_back(makeTokenNode(name));
  }
  return std::move(vec);
}

DEFINE_CB(clangStmtProc) {
  //ClangStmtHandler.walk(node, w, src, ss);
}

DEFINE_CB(emitClassDefinition) {
  const auto typeName = getProp(node, "type");
  const auto type = src.typeTable.at(typeName);
  XcodeMl::ClassType* classType =
    llvm::cast<XcodeMl::ClassType>(type.get());
  auto declNode = classType->getNode();
  if (!declNode) {
    return  makeTokenNode("class") +
            classType->name() +
            makeTokenNode(";") +
            makeNewLineNode();
  }
  return w.walk(declNode, src) + makeTokenNode(";");
}

DEFINE_CB(functionDefinitionProc) {
  xmlNodePtr nameElem = findFirst(
      node,
      "name|operator|constructor|destructor",
      src.ctxt
  );
  const XMLString name(xmlNodeGetContent(nameElem));
  const XMLString kind(nameElem->name);
  auto nameNode =
    makeTokenNode(
    (kind == "name" || kind == "operator") ?
      name :
      (kind == "constructor") ?
        "<CONSTRUCTOR>" :
        (kind == "destructor") ?
          "<DESTRUCTOR>" :
          throw);

  auto fnTypeName = findSymbolType(src.symTable, name);
  auto fnType = llvm::cast<XcodeMl::Function>(
      src.typeTable.at(fnTypeName).get());
  auto acc =
    fnType->makeDeclaration(
        nameNode,
        getParams(node, src),
        src.typeTable);

  acc = acc + makeTokenNode( "{" ) + makeNewLineNode();
  acc = acc + makeInnerNode(w.walkChildren(node, src));
  return acc + makeTokenNode("}");
}

DEFINE_CB(functionDeclProc) {
  const auto name = getNameFromIdNode(node, src.ctxt);
  assert(!name.empty());
  const auto fnType = getIdentType(src, name);
  return
    makeDecl(
        fnType,
        makeTokenNode(name),
        src.typeTable) +
    makeTokenNode(";");
}

DEFINE_CB(memberRefProc) {
  return
    makeInnerNode(w.walkChildren(node, src)) +
    makeTokenNode(".") +
    makeTokenNode(getProp(node, "member"));
}

DEFINE_CB(memberAddrProc) {
  return
    makeTokenNode("&") +
    memberRefProc(w, node, src);
}

DEFINE_CB(memberPointerRefProc) {
  return
    makeInnerNode(w.walkChildren(node, src)) +
    makeTokenNode(".*") +
    makeTokenNode(getProp(node, "name"));
}

DEFINE_CB(compoundValueProc) {
  return
    makeTokenNode("{") +
    makeInnerNode(w.walkChildren(node, src)) +
    makeTokenNode("}");
}

DEFINE_CB(thisExprProc) {
  return makeTokenNode("this");
}

const auto compoundStatementProc = handleScope;

DEFINE_CB(whileStatementProc) {
  auto cond = findFirst(node, "condition", src.ctxt),
       body = findFirst(node, "body", src.ctxt);
  return
    makeTokenNode("while") +
    makeTokenNode("(") +
    w.walk(cond, src) +
    makeTokenNode(")") +
    handleScope(w, body, src);
}

DEFINE_CB(doStatementProc) {
  auto cond = findFirst(node, "condition", src.ctxt),
       body = findFirst(node, "body", src.ctxt);
  return
    makeTokenNode("do") +
    handleScope(w, body, src) +
    makeTokenNode("while") +
    makeTokenNode("(") +
    w.walk(cond, src) +
    makeTokenNode(")") +
    makeTokenNode(";");
}

DEFINE_CB(forStatementProc) {
  auto init = findFirst(node, "init", src.ctxt),
       cond = findFirst(node, "condition", src.ctxt),
       iter = findFirst(node, "iter", src.ctxt),
       body = findFirst(node, "body", src.ctxt);
  auto acc = makeTokenNode("for") + makeTokenNode("(");
  if (init) {
    acc = acc + w.walk(init, src);
  }
  acc = acc + makeTokenNode(";");
  if (cond) {
    acc = acc + w.walk(cond, src);
  }
  acc = acc + makeTokenNode(";");
  if (iter) {
    acc = acc + w.walk(iter, src);
  }
  acc = acc + makeTokenNode(")");
  return acc + handleScope(w, body, src);
}

DEFINE_CB(returnStatementProc) {
  xmlNodePtr child = xmlFirstElementChild(node);
  if (child) {
    return
      makeTokenNode("return") +
      w.walk(child, src) +
      makeTokenNode(";");
  } else {
    return
      makeTokenNode("return") +
      makeTokenNode(";");
  }
}

DEFINE_CB(exprStatementProc) {
  const auto showChild = showChildElem("", ";");
  return showChild(w, node, src);
}

DEFINE_CB(functionCallProc) {
  xmlNodePtr function = findFirst(node, "function/*", src.ctxt);
  xmlNodePtr arguments = findFirst(node, "arguments", src.ctxt);
  return w.walk(function, src) + w.walk(arguments, src);
}

DEFINE_CB(argumentsProc) {
  auto acc = makeTokenNode("(");
  bool alreadyPrinted = false;
  for (xmlNodePtr arg = node->children; arg; arg = arg->next) {
    if (arg->type != XML_ELEMENT_NODE) {
      continue;
    }
    if (alreadyPrinted) {
      acc = acc + makeTokenNode(",");
    }
    acc = acc + w.walk(arg, src);
    alreadyPrinted = true;
  }
  return acc + makeTokenNode(")");
}

DEFINE_CB(condExprProc) {
  xmlNodePtr prd = findFirst(node, "*[1]", src.ctxt),
             the = findFirst(node, "*[2]", src.ctxt),
             els = findFirst(node, "*[3]", src.ctxt);
  return
    w.walk(prd, src) +
    makeTokenNode("?") +
    w.walk(the, src) +
    makeTokenNode(":") +
    w.walk(els, src);
}

DEFINE_CB(varDeclProc) {
  xmlNodePtr nameElem = findFirst(node, "name|operator", src.ctxt);
  XMLString name(xmlNodeGetContent(nameElem));
  assert(length(name) != 0);
  auto type = getIdentType(src, name);
  auto acc = makeDecl(
      type,
      makeTokenNode(name),
      src.typeTable);
  xmlNodePtr valueElem = findFirst(node, "value", src.ctxt);
  if (valueElem) {
    acc = acc + makeTokenNode("=") + w.walk(valueElem, src);
  }
  return acc + makeTokenNode(";");
}

const CodeBuilder CXXBuilder(
"CodeBuilder",
makeInnerNode,
{
  //{ "clangStmt", clangStmtProc },

  { "typeTable", NullProc },
  { "functionDefinition", functionDefinitionProc },
  { "functionDecl", functionDeclProc },
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
  { "logEQExpr", showBinOp(" == ")},
  { "unaryMinusExpr", showUnaryOp("-") },
  { "binNotExpr", showUnaryOp("~") },
  { "logNotExpr", showUnaryOp("!") },
  { "sizeOfExpr", showUnaryOp("sizeof") },
  { "functionCall", functionCallProc },
  { "arguments", argumentsProc },
  { "condExpr", condExprProc },
  { "exprStatement", exprStatementProc },
  { "returnStatement", returnStatementProc },
  { "varDecl", varDeclProc },
  { "classDecl", emitClassDefinition },

  /* for CtoXcodeML */
  { "Decl_Record", NullProc },
    // Ignore Decl_Record (structs are already emitted)
});

/*!
 * \brief Traverse an XcodeML document and generate C++ source code.
 * \param[in] doc XcodeML document.
 * \param[out] ss Stringstream to flush C++ source code.
 */
void buildCode(
    xmlNodePtr rootNode,
    xmlXPathContextPtr ctxt,
    std::stringstream& ss
) {
  SourceInfo src = {
    ctxt,
    parseTypeTable(rootNode, ctxt, ss),
    parseGlobalSymbols(rootNode, ctxt, ss)
  };

  cxxgen::Stream out;
  xmlNodePtr globalSymbols = findFirst(rootNode, "/XcodeProgram/globalSymbols", src.ctxt);
  buildSymbols(globalSymbols, src)
    ->flush(out);

  xmlNodePtr globalDeclarations =
    findFirst(rootNode, "/XcodeProgram/globalDeclarations", src.ctxt);
  separateByBlankLines(
      CXXBuilder.walkChildren(globalDeclarations, src))
    ->flush(out);

  ss << out.str();
}
