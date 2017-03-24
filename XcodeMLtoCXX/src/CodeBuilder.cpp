#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "AttrProc.h"
#include "Stream.h"
#include "StringTree.h"
#include "Symbol.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "NnsAnalyzer.h"
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

using cxxgen::insertNewLines;
using cxxgen::separateByBlankLines;

static XcodeMl::CodeFragment
getDeclNameFromTypedNode(
    xmlNodePtr node,
    const SourceInfo& src)
{
  if (findFirst(node, "constructor", src.ctxt)) {
    const auto classDtident = getProp(node, "parent_class");
    const auto T = src.typeTable[classDtident];
    const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
    const auto name = classT->name();
    assert(name.hasValue());
    return *name;
  } else if (findFirst(node, "destructor", src.ctxt)) {
    const auto classDtident = getProp(node, "parent_class");
    const auto T = src.typeTable[classDtident];
    const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
    const auto name = classT->name();
    assert(name.hasValue());
    return makeTokenNode("~") + (*name);
  }
  return makeTokenNode(getNameFromIdNode(node, src.ctxt));
}

static XcodeMl::CodeFragment
getQualifiedNameFromTypedNode(
    xmlNodePtr node,
    const SourceInfo& src)
{
  const auto name = getDeclNameFromTypedNode(node, src);
  auto nameNode = findFirst(node, "name", src.ctxt);
  const auto ident = getPropOrNull(nameNode, "nns");
  if (ident.hasValue()) {
    const auto nns = src.nnsTable.at(*ident);
    return nns->makeDeclaration(src.typeTable, src.nnsTable) + name;
  } else {
    return name;
  }
}

static XcodeMl::CodeFragment
wrapWithLangLink(
    const XcodeMl::CodeFragment& content,
    xmlNodePtr node)
{
  const auto lang = getPropOrNull(node, "language_linkage");
  if (! lang.hasValue() || *lang == "C++") {
    return content;
  } else {
    return
      makeTokenNode("extern") +
      makeTokenNode("\"" + *lang + "\"") +
      makeTokenNode("{") +
      content +
      makeTokenNode("}");
  }
}

static XcodeMl::CodeFragment
makeNestedNameSpec(
    const XcodeMl::NnsRef& nns,
    const SourceInfo& src)
{
  return nns->makeDeclaration(src.typeTable, src.nnsTable);
}

static XcodeMl::CodeFragment
makeNestedNameSpec(
    const std::string& ident,
    const SourceInfo& src)
{
  const auto nns = src.nnsTable.at(ident);
  return makeNestedNameSpec(nns, src);
}

static bool
isInClassDecl(xmlNodePtr node, const SourceInfo& src) {
  // FIXME: temporary implementation
  auto parent = findFirst(node, "..", src.ctxt);
  if (!parent) {
    return false;
  }
  const auto name = static_cast<XMLString>(parent->name);
  return name == "classDecl";
}

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
    xmlNodePtr nameElem = findFirst(idElem, "name|operator", ctxt);
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
  log << std::endl << "{" << std::endl;
  for (auto entry : table) {
    log << "\t{";
    for (auto p : entry) {
      log << "(" << p.first << "," << p.second << "),";
    }
    log << "}" << std::endl;
  }
  log << "}" << std::endl;
  throw std::runtime_error(
      name + " not found in SymbolMap: " + log.str());
}

static std::string
getDtidentFromTypedNode(
    xmlNodePtr node,
    xmlXPathContextPtr ctxt,
    const SymbolMap& table)
{
  const auto dtident = getPropOrNull(node, "type");
  if (dtident.hasValue()) {
    return *dtident;
  } else {
    const auto name = getNameFromIdNode(node, ctxt);
    return findSymbolType(table, name);
  }
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

static cxxgen::StringTreeRef
wrapWithParen(const cxxgen::StringTreeRef& str) {
  return makeTokenNode("(")
    + str
    + makeTokenNode(")");
}

static cxxgen::StringTreeRef
foldWithSemicolon(const std::vector<StringTreeRef>& stmts) {
  auto node = makeVoidNode();
  for (auto& stmt : stmts) {
    node = node + stmt + makeTokenNode(";") + makeNewLineNode();
  }
  return node;
}

DEFINE_CB(walkChildrenWithInsertingNewLines) {
  return foldWithSemicolon( w.walkChildren(node, src) );
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
  return showChildElem(
      std::string("(") + Operator + "(",
      "))");
}

DEFINE_CB(postIncrExprProc) {
  return
    makeTokenNode("(") +
    makeInnerNode(w.walkChildren(node, src)) +
    makeTokenNode("++)");
}

DEFINE_CB(postDecrExprProc) {
  return
    makeTokenNode("(") +
    makeInnerNode(w.walkChildren(node, src)) +
    makeTokenNode("--)");
}

DEFINE_CB(castExprProc) {
  const auto dtident = getProp(node, "type");
  const auto Tstr = makeDecl(
      src.typeTable.at(dtident),
      makeVoidNode(),
      src.typeTable);
  const auto child = makeInnerNode(w.walkChildren(node, src));
  return wrapWithParen(wrapWithParen(Tstr) + wrapWithParen(child));
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

static SymbolEntry
ClassSymbolsToSymbolEntry(const XcodeMl::ClassType* T) {
  using namespace XcodeMl;
  assert(T);
  SymbolEntry entry;
  auto members = T->getSymbols();
  for (auto&& member : members) {
    ClassType::MemberName name; DataTypeIdent dtident;
    std::tie(name, dtident) = member;
    entry[name] = dtident;
  }
  return std::move(entry);
}

static XcodeMl::CodeFragment
makeBases(
    XcodeMl::ClassType *T,
    SourceInfo& src)
{
  using namespace XcodeMl;
  assert(T);
  const auto bases = T->getBases();
  std::vector<CodeFragment> decls;
  std::transform(
      bases.begin(),
      bases.end(),
      std::back_inserter(decls),
      [&src] (ClassType::BaseClass base) {
        const auto T = src.typeTable.at(std::get<1>(base));
        const auto classT = llvm::cast<ClassType>(T.get());
        assert(classT);
        assert(classT->name().hasValue());
        return
          makeTokenNode(std::get<0>(base)) +
          makeTokenNode(std::get<2>(base) ? "virtual":"") +
          *(classT->name());
      });
  return
    decls.empty() ?
      makeVoidNode()
    : makeTokenNode(":") + cxxgen::join(",", decls);
}

DEFINE_CB(emitClassDefinition) {
  if (isTrueProp(node, "is_implicit", false)) {
    return makeVoidNode();
  }
  const auto typeName = getProp(node, "type");
  const auto type = src.typeTable.at(typeName);
  auto classType = llvm::dyn_cast<XcodeMl::ClassType>(type.get());
  assert(classType);
  const auto className = classType->name();

  src.symTable.push_back(ClassSymbolsToSymbolEntry(classType));
  std::vector<XcodeMl::CodeFragment> decls;

  for (xmlNodePtr memberNode = xmlFirstElementChild(node);
       memberNode;
       memberNode = xmlNextElementSibling(memberNode)) {
    const auto accessProp = getPropOrNull(memberNode, "access");
    if (!accessProp.hasValue()) {
      return
        makeTokenNode("/* ignored a member with no access specifier */") +
        makeNewLineNode();
    }
    const auto access = *accessProp;
    const auto decl =
      makeTokenNode(access) +
      makeTokenNode(":") +
      w.walk(memberNode, src);
    decls.push_back(decl);
  }

  src.symTable.pop_back();

  return
    makeTokenNode("class") +
    (className.hasValue() ? *className : makeVoidNode()) +
    makeBases(classType, src) +
    makeTokenNode("{") +
    separateByBlankLines(decls) +
    makeTokenNode("}") +
    makeTokenNode(";") +
    makeNewLineNode();
}

DEFINE_CB(classDeclProc) {
  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    return emitClassDefinition(w, node, src);
  }
  const auto T = src.typeTable.at(getProp(node, "type"));
  auto classT = llvm::dyn_cast<XcodeMl::ClassType>(T.get());
  assert(classT);
  const auto name = classT->name();
  assert(name.hasValue());
  return
    makeTokenNode("class") +
    (*name) +
    makeTokenNode(";");
}

static XcodeMl::CodeFragment
makeFunctionDeclHead(
    xmlNodePtr node,
    const std::vector<XcodeMl::CodeFragment> args,
    const SourceInfo& src) {
  xmlNodePtr nameElem = findFirst(
      node,
      "name|operator|constructor|destructor",
      src.ctxt
  );
  const XMLString name(xmlNodeGetContent(nameElem));
  const XMLString kind(nameElem->name);
  const auto nameNode = getQualifiedNameFromTypedNode(node, src);

  const auto dtident = getDtidentFromTypedNode(
      node,
      src.ctxt,
      src.symTable);
  const auto T = src.typeTable[dtident];
  const auto fnType = llvm::cast<XcodeMl::Function>(T.get());
  auto acc = makeVoidNode();
  if (isInClassDecl(node, src)
      && isTrueProp(node, "is_virtual", false))
  {
    acc = acc + makeTokenNode("virtual");
  }
  acc = acc +
    (kind == "constructor" || kind == "destructor" ?
      fnType->makeDeclarationWithoutReturnType(
          nameNode,
          args,
          src.typeTable)
    : fnType->makeDeclaration(
          nameNode,
          args,
          src.typeTable));
  return acc;
}

DEFINE_CB(functionDefinitionProc) {
  const auto args = getParams(node, src);
  auto acc =
    makeFunctionDeclHead(node, args, src);

  if (auto ctorInitList = findFirst(
        node,
        "constructorInitializerList",
        src.ctxt))
  {
    acc = acc + w.walk(ctorInitList, src);
  }

  auto body = findFirst(node, "body", src.ctxt);
  assert(body);
  acc = acc + makeTokenNode( "{" ) + makeNewLineNode();
  acc = acc + w.walk(body, src);
  acc = acc + makeTokenNode("}");
  return wrapWithLangLink(acc, node);
}

DEFINE_CB(functionDeclProc) {
  const auto fnDtident = getDtidentFromTypedNode(
      node,
      src.ctxt,
      src.symTable);
  const auto fnType =
    llvm::cast<XcodeMl::Function>(src.typeTable[fnDtident].get());
  auto decl =
    makeFunctionDeclHead(node, fnType->argNames(), src) +
    makeTokenNode(";");
  return wrapWithLangLink(decl, node);
}

DEFINE_CB(memberRefProc) {
  const auto baseName = getProp(node, "member");
  const auto nnsident = getPropOrNull(node, "nns");
  const auto name =
    (nnsident.hasValue() ?
        makeNestedNameSpec(*nnsident, src)
      : makeVoidNode()) +
    makeTokenNode(baseName);
  return
    makeInnerNode(w.walkChildren(node, src)) +
    makeTokenNode("->") +
    name;
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

DEFINE_CB(ifStatementProc) {
  auto cond = findFirst(node, "condition", src.ctxt),
       thenpart = findFirst(node, "then", src.ctxt),
       elsepart = findFirst(node, "else", src.ctxt);
  auto acc = makeTokenNode("if") + makeTokenNode("(");
  if (cond) {
    acc = acc + w.walk(cond, src);
  }
  acc = acc + makeTokenNode(") {");
  if (thenpart) {
    acc = acc + handleScope(w, thenpart, src);
  }
  if (elsepart) {
    acc = acc + makeTokenNode("} else {");
    acc = acc + handleScope(w, elsepart, src);
  }
  acc = acc + makeTokenNode("}");
  return acc;
}

DEFINE_CB(switchStatementProc) {
  auto cond = findFirst(node, "value", src.ctxt);
  auto body = findFirst(node, "body", src.ctxt);
  return makeTokenNode("switch")
    + wrapWithParen(w.walk(cond, src))
    + makeTokenNode("{")
    + insertNewLines(w.walkChildren(body, src))
    + makeTokenNode("}");
}

DEFINE_CB(caseLabelProc) {
  auto value = findFirst(node, "value", src.ctxt);
  return makeTokenNode("case")
    + makeInnerNode(w.walkChildren(value, src))
    + makeTokenNode(":");
}

DEFINE_CB(defaultLabelProc) {
  return makeTokenNode("default") + makeTokenNode(":");
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
          second = findFirst(node, "*[2]", src.ctxt),
           third = findFirst(node, "*[3]", src.ctxt);
  if (third) {
    return
      makeTokenNode( "(" ) +
      w.walk(prd, src) +
      makeTokenNode( "?" ) +
      w.walk(second, src) +
      makeTokenNode( ":" ) +
      w.walk(third, src) +
      makeTokenNode(")");
  } else {
    return
      makeTokenNode( "(" ) +
      w.walk(prd, src) +
      makeTokenNode( "?:" ) +
      w.walk(second, src) +
      makeTokenNode(")");
  }
}

DEFINE_CB(addrOfExprProc) {
  if (isTrueProp(node, "is_expedient", false)) {
    auto expr = findFirst(node, "*[1]", src.ctxt);
    return w.walk(expr, src);
  }
  const auto wrap = showUnaryOp("&");
  return wrap(w, node, src);
}

DEFINE_CB(varDeclProc) {
  xmlNodePtr nameElem = findFirst(node, "name|operator", src.ctxt);
  XMLString name(xmlNodeGetContent(nameElem));
  assert(length(name) != 0);
  auto type = getIdentType(src, name);
  auto acc = makeVoidNode();
  if (isInClassDecl(node, src)
      && isTrueProp(node, "is_static_data_member", false))
  {
    acc = acc + makeTokenNode("static");
  }
  acc = acc
    + makeDecl(
        type,
        makeTokenNode(name),
        src.typeTable);
  xmlNodePtr valueElem = findFirst(node, "value", src.ctxt);
  if (!valueElem) {
    return wrapWithLangLink(acc + makeTokenNode(";"), node);
  }

  if (auto ctorExpr = findFirst(
        valueElem,
        "clangStmt[@class='CXXConstructExpr']",
        src.ctxt))
  {
    const auto args = w.walkChildren(ctorExpr, src);
    /*
     * X a();  // illegal ([dcl.init]/6)
     * X a;    // OK
     */
    const auto init =
      args.empty() ?
        makeVoidNode()
      : makeTokenNode("(") +
        cxxgen::join(",", args) +
        makeTokenNode(")");
    acc =
      acc +
      init +
      makeTokenNode(";");
    return wrapWithLangLink(acc, node);
  }

  acc = acc
    + makeTokenNode("=")
    + w.walk(valueElem, src)
    + makeTokenNode(";");
  return wrapWithLangLink(acc, node);
}

DEFINE_CB(usingDeclProc) {
  const auto name = getQualifiedNameFromTypedNode(node, src);
    // FIXME: using declaration of base constructor
  const auto head =
    isTrueProp(node, "is_access_declaration", false) ?
      makeVoidNode()
    : makeTokenNode("using");
  return head + name + makeTokenNode(";");
}

DEFINE_CB(ctorInitListProc) {
  auto inits = findNodes(node, "constructorInitializer", src.ctxt);
  if (inits.empty()) {
    return makeVoidNode();
  }
  auto decl = makeVoidNode();
  bool alreadyPrinted = false;
  for (auto init : inits) {
    decl = decl
      + makeTokenNode(alreadyPrinted ? "," : ":")
      + w.walk(init, src);
    alreadyPrinted = true;
  }
  return decl;
}

static XcodeMl::CodeFragment
getCtorInitName(
    xmlNodePtr node,
    const XcodeMl::Environment& env)
{
  const auto dataMember = getPropOrNull(node, "member");
  if (dataMember.hasValue()) {
    return makeTokenNode(*dataMember);
  }
  const auto base = getPropOrNull(node, "type");
  if (base.hasValue()) {
    const auto T = env[*base];
    const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
    const auto name = classT->name();
    assert(name.hasValue());
    return *name;
  }

  xmlDebugDumpNode(stderr, node, 0);
  assert(false);
}

DEFINE_CB(ctorInitProc) {
  const auto member = getCtorInitName(node, src.typeTable);
  auto expr = findFirst(node, "*[1]", src.ctxt);
  assert(expr);
  return member +
    makeTokenNode("(") +
    w.walk(expr, src) +
    makeTokenNode(")");
}

DEFINE_CB(clangStmtProc) {
  return ClangStmtHandler.walk(node, w, src);
}

DEFINE_CB(clangDeclProc) {
  return ClangDeclHandler.walk(node, w, src);
}

const CodeBuilder CXXBuilder(
"CodeBuilder",
makeInnerNode,
{
  { "typeTable", NullProc },
  { "functionDefinition", functionDefinitionProc },
  { "functionDecl", functionDeclProc },
  { "intConstant", EmptySNCProc },
  { "moeConstant", EmptySNCProc },
  { "booleanConstant", EmptySNCProc },
  { "funcAddr", EmptySNCProc },
  { "stringConstant", showNodeContent("\"", "\"") },
  { "Var", EmptySNCProc },
  { "varAddr", showNodeContent("(&", ")") },
  { "pointerRef", showUnaryOp("*") },
  { "memberRef", memberRefProc },
  { "memberAddr", memberAddrProc },
  { "memberPointerRef", memberPointerRefProc },
  { "compoundValue", compoundValueProc },
  { "compoundStatement", compoundStatementProc },
  { "whileStatement", whileStatementProc },
  { "doStatement", doStatementProc },
  { "forStatement", forStatementProc },
  { "ifStatement", ifStatementProc },
  { "switchStatement" , switchStatementProc },
  { "caseLabel", caseLabelProc },
  { "defaultLabel", defaultLabelProc },
  { "thisExpr", thisExprProc },
  { "assignExpr", showBinOp(" = ") },
  { "plusExpr", showBinOp(" + ") },
  { "minusExpr", showBinOp(" - ") },
  { "mulExpr", showBinOp(" * ") },
  { "divExpr", showBinOp(" / ") },
  { "modExpr", showBinOp(" % ") },
  { "LshiftExpr", showBinOp(" << ")},
  { "RshiftExpr", showBinOp(" >> ")},
  { "logLTExpr", showBinOp(" < ")},
  { "logGTExpr", showBinOp(" > ")},
  { "logLEExpr", showBinOp(" <= ")},
  { "logGEExpr", showBinOp(" >= ")},
  { "logEQExpr", showBinOp(" == ")},
  { "logNEQExpr", showBinOp(" != ")},
  { "bitAndExpr", showBinOp(" & ")},
  { "bitXorExpr", showBinOp(" ^ ")},
  { "bitOrExpr", showBinOp(" | ")},
  { "logAndExpr", showBinOp(" && ")},
  { "logOrExpr", showBinOp(" || ")},
  { "asgMulExpr", showBinOp(" *= ")},
  { "asgDivExpr", showBinOp(" /= ")},
  { "asgPlusExpr", showBinOp(" += ")},
  { "asgMinusExpr", showBinOp(" -= ")},
  { "asgLshiftExpr", showBinOp(" <<= ")},
  { "asgRshiftExpr", showBinOp(" >>= ")},
  { "asgBitAndExpr", showBinOp(" &= ")},
  { "asgBitOrExpr", showBinOp(" |= ")},
  { "asgBitXorExpr", showBinOp(" ^= ")},
  { "unaryPlusExpr", showUnaryOp("+") },
  { "unaryMinusExpr", showUnaryOp("-") },
  { "preIncrExpr", showUnaryOp("++") },
  { "preDecrExpr", showUnaryOp("--") },
  { "postIncrExpr", postIncrExprProc },
  { "postDecrExpr", postDecrExprProc },
  { "castExpr", castExprProc },
  { "AddrOfExpr", addrOfExprProc },
  { "pointerRef", showUnaryOp("*") },
  { "bitNotExpr", showUnaryOp("~") },
  { "logNotExpr", showUnaryOp("!") },
  { "sizeOfExpr", showUnaryOp("sizeof") },
  { "functionCall", functionCallProc },
  { "arguments", argumentsProc },
  { "condExpr", condExprProc },
  { "exprStatement", exprStatementProc },
  { "returnStatement", returnStatementProc },
  { "varDecl", varDeclProc },
  { "usingDecl", usingDeclProc },
  { "classDecl", classDeclProc },

  /* out of specification */
  { "constructorInitializer", ctorInitProc },
  { "constructorInitializerList", ctorInitListProc },

  /* for elements defined by clang */
  { "clangStmt", clangStmtProc },
  { "clangDecl", clangDeclProc },

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

  xmlNodePtr typeTableNode =
    findFirst(rootNode, "/XcodeProgram/typeTable", ctxt);
  xmlNodePtr nnsTableNode =
    findFirst(rootNode, "/XcodeProgram/nnsTable", ctxt);
  SourceInfo src = {
    ctxt,
    parseTypeTable(typeTableNode, ctxt, ss),
    parseGlobalSymbols(rootNode, ctxt, ss),
    analyzeNnsTable(nnsTableNode, ctxt),
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
