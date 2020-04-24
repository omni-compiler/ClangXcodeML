#include <algorithm>
#include <functional>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <string>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "StringTree.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "AttrProc.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"
#include "XcodeMlType.h"
#include "XcodeMlTypeTable.h"
#include "XcodeMlOperator.h"
#include "SourceInfo.h"
#include "TypeAnalyzer.h"
#include "NnsAnalyzer.h"
#include "CodeBuilder.h"
#include "ClangStmtHandler.h"
#include "XcodeMlUtil.h"
#include "ClangNestedNameSpecHandler.h"

namespace cxxgen = CXXCodeGen;

#define STMTHANDLER_ARGS                                                      \
  xmlNodePtr node __attribute__((unused)),                                    \
      const CodeBuilder &w __attribute__((unused)),                           \
      SourceInfo &src __attribute__((unused))

#define DEFINE_STMTHANDLER(name) XcodeMl::CodeFragment name(STMTHANDLER_ARGS)

using cxxgen::makeInnerNode;
using cxxgen::makeTokenNode;
using XcodeMl::CodeFragment;

namespace {

CodeFragment
createNode(xmlNodePtr node,
    const char *xpath,
    const CodeBuilder &w,
    SourceInfo &src) {
  const auto targetNode = findFirst(node, xpath, src.ctxt);
  if (!targetNode) {
    std::cerr << "In createNode" << std::endl
	      << "Line" << xmlGetLineNo(node) << std::endl
              << "not found: '" << xpath << "'" << std::endl;
    throw(std::runtime_error("Node CreateFailed"));
  }
  return w.walk(targetNode, src);
}

CodeFragment
createNodeOrNull(xmlNodePtr node,
    const char *xpath,
    const CodeBuilder &w,
    SourceInfo &src) {
  const auto targetNode = findFirst(node, xpath, src.ctxt);
  if (!targetNode) {
    return CXXCodeGen::makeVoidNode();
  }
  return w.walk(targetNode, src);
}

std::vector<CodeFragment>
createNodes(xmlNodePtr node,
    const char *xpath,
    const CodeBuilder &w,
    SourceInfo &src) {
  std::vector<CodeFragment> vec;
  const auto targetNodes = findNodes(node, xpath, src.ctxt);
  for (auto &&targetNode : targetNodes) {
    vec.push_back(w.walk(targetNode, src));
  }
  return vec;
}

CodeFragment
makeCompoundStmt(const CodeFragment &stmt) {
  // no STMTHANDLER returns a code fragment that ends with a semicolon.
  return wrapWithBrace(stmt + makeTokenNode(";"));
}

DEFINE_STMTHANDLER(callCodeBuilder) {
  return makeInnerNode(ProgramBuilder.walkChildren(node, src));
}

DEFINE_STMTHANDLER(ArraySubscriptExprProc) {
  const auto array = createNode(node, "clangStmt[1]", w, src);
  const auto index = createNode(node, "clangStmt[2]", w, src);
  return wrapWithParen(array) + wrapWithSquareBracket(index);
}

DEFINE_STMTHANDLER(BinaryConditionalOperatorProc) {
  const auto lhs = createNode(node, "clangStmt[1]", w, src);
  const auto rhs = createNode(node, "clangStmt[4]", w, src);
  return wrapWithParen(lhs + makeTokenNode("?:") + rhs);
}

DEFINE_STMTHANDLER(BinaryOperatorProc) {
  const auto lhsNode = findFirst(node, "clangStmt[1]", src.ctxt);
  const auto lhs = w.walk(lhsNode, src);
  const auto rhsNode = findFirst(node, "clangStmt[2]", src.ctxt);
  const auto rhs = w.walk(rhsNode, src);

  const auto opName = getProp(node, "binOpName");
  const auto opSpelling = XcodeMl::OperatorNameToSpelling(opName);
  if (!opSpelling.hasValue()) {
    std::cerr << "Unknown Binary operator name: '" << opName << "'" << xmlGetLineNo(node)<< std::endl;
    std::abort();
  }
  return wrapWithParen(lhs + makeTokenNode(*opSpelling) + rhs);
}

DEFINE_STMTHANDLER(BreakStmtProc) {
  return makeTokenNode("break");
}

DEFINE_STMTHANDLER(callExprProc) {
  const auto funcNode = findFirst(node, "clangStmt", src.ctxt);
  const auto func = w.walk(funcNode, src);

  const auto argNodes = findNodes(node, "clangStmt[position() > 1]", src.ctxt);
  std::vector<XcodeMl::CodeFragment> args;
  for (auto &&argNode : argNodes) {
    args.push_back(w.walk(argNode, src));
  }

  return wrapWithParen(func + wrapWithParen(join(",", args)));
}

DEFINE_STMTHANDLER(CaseStmtProc) {
  const auto labelNode = findFirst(node, "clangStmt[1]", src.ctxt);
  const auto label = w.walk(labelNode, src);

  const auto bodyNode = findFirst(node, "clangStmt[2]", src.ctxt);
  const auto body = w.walk(bodyNode, src);

  return makeTokenNode("case") + label + makeTokenNode(":") + body;
}

DEFINE_STMTHANDLER(CompoundStmtProc) {
  const auto stmtNodes = findNodes(node, "clangStmt", src.ctxt);
  std::vector<CXXCodeGen::StringTreeRef> stmts;
  for (auto &&stmtNode : stmtNodes) {
    stmts.push_back(w.walk(stmtNode, src));
  }
  return wrapWithBrace(foldWithSemicolon(stmts));
}

DEFINE_STMTHANDLER(ConditionalOperatorProc) {
  const auto cond = createNode(node, "clangStmt[1]", w, src);
  const auto yes = createNode(node, "clangStmt[2]", w, src);
  const auto no = createNode(node, "clangStmt[3]", w, src);
  return wrapWithParen(
      cond + makeTokenNode("?") + yes + makeTokenNode(":") + no);
}

DEFINE_STMTHANDLER(ContinueStmtProc) {
  return makeTokenNode("continue");
}

DEFINE_STMTHANDLER(CStyleCastExprProc) {
  const auto type = createNode(node, "clangTypeLoc", w, src);
  const auto expr = createNode(node, "clangStmt", w, src);
  return wrapWithParen(wrapWithParen(type) + expr);
}

DEFINE_STMTHANDLER(CXXBoolLiteralExprProc) {
  if (isTrueProp(node, "bool_value", false)) {
    return makeTokenNode("true");
  } else {
    return makeTokenNode("false");
  }
}

DEFINE_STMTHANDLER(CXXCatchStmtProc) {
  const auto body = createNode(node, "clangStmt", w, src);
  if (const auto declNode = findFirst(node, "clangDecl", src.ctxt)) {
    const auto name =
        getQualifiedName(declNode, src).toString(src.typeTable, src.nnsTable);
    const auto dtident = getProp(declNode, "xcodemlType");
    const auto T = src.typeTable.at(dtident);
    const auto var = makeDecl(T, name, src.typeTable, src.nnsTable);
    return makeTokenNode("catch") + wrapWithParen(var) + body;
  } else {
    return makeTokenNode("catch(...)") + body;
  }
}

DEFINE_STMTHANDLER(CXXConstCastExprProc) {
  const auto type = createNode(node, "clangTypeLoc", w, src);
  const auto expr = createNode(node, "clangStmt", w, src);
  return makeTokenNode("const_cast") + makeTokenNode("<") + type
      + makeTokenNode(">") + wrapWithParen(expr);
}

DEFINE_STMTHANDLER(CXXCtorExprProc) {
  auto materializeExpr = findFirst(
      node, "clangStmt[@class='MaterializeTemporaryExpr']", src.ctxt);
  if (materializeExpr) {
    return w.walk(materializeExpr, src);
  }
  auto child = findFirst(node, "clangStmt", src.ctxt);
  if (child && getType(child) == getType(node)
      && !findFirst(node, "clangStmt[position() > 1]", src.ctxt)) {
    // this is a copy (or move) constructor: omit this
    return w.walk(child, src);
  }

  const auto T = makeDecl(src.typeTable.at(getType(node)),
      CXXCodeGen::makeVoidNode(),
      src.typeTable,
      src.nnsTable);
  const auto exprs = createNodes(node, "clangStmt", w, src);
  return wrapWithXcodeMlIdentity(T) + wrapWithParen(join(",", exprs));
}

DEFINE_STMTHANDLER(CXXDeleteExprProc) {
  const auto is_global = isTrueProp(node, "is_global_delete", false);
  const auto is_array = isTrueProp(node, "is_array_form", false);
  const auto allocated = findFirst(node, "*", src.ctxt);
  return (is_global ? makeTokenNode("::") : CXXCodeGen::makeVoidNode())
      + makeTokenNode(is_array ? "delete[]" : "delete")
      + w.walk(allocated, src);
}

DEFINE_STMTHANDLER(CXXDynamicCastExprProc) {
  const auto type = createNode(node, "clangTypeLoc", w, src);
  const auto expr = createNode(node, "clangStmt", w, src);
  return makeTokenNode("dynamic_cast") + makeTokenNode("<") + type
      + makeTokenNode(">") + wrapWithParen(expr);
}

DEFINE_STMTHANDLER(emitNewArrayExpr) {
  const auto is_global = isTrueProp(node, "is_global_new", false);
  const auto T = src.typeTable.at(getType(node));
  const auto pointeeT =
      llvm::cast<XcodeMl::Pointer>(T.get())->getPointee(src.typeTable);
  const auto NewTypeId = pointeeT->makeDeclaration(
      CXXCodeGen::makeVoidNode(), src.typeTable, src.nnsTable);
  const auto type = wrapWithXcodeMlIdentity(NewTypeId);

  const auto size = createNode(node, "clangStmt[1]", w, src);

  if (isTrueProp(node, "has_initializer", false)) {
    const auto initNode = findFirst(node, "clangStmt[2]", src.ctxt);
    const auto init = initNode
        ? wrapWithParen(join(",", w.walkChildren(initNode, src)))
        : CXXCodeGen::makeVoidNode();

    const auto placementArgs =
        createNodes(node, "clangStmt[position() > 2]", w, src);
    const auto placementArgSequence = placementArgs.empty()
        ? CXXCodeGen::makeVoidNode()
        : wrapWithParen(join(",", placementArgs));

    return (is_global ? makeTokenNode("::") : CXXCodeGen::makeVoidNode())
        + makeTokenNode("new") + placementArgSequence + type
        + wrapWithSquareBracket(size) + init;
  } else {
    const auto placementArgs =
        createNodes(node, "clangStmt[position() > 1]", w, src);
    const auto placementArgSequence = placementArgs.empty()
        ? CXXCodeGen::makeVoidNode()
        : wrapWithParen(join(",", placementArgs));

    return (is_global ? makeTokenNode("::") : CXXCodeGen::makeVoidNode())
        + makeTokenNode("new") + placementArgSequence + type
        + wrapWithSquareBracket(size);
  }
}

DEFINE_STMTHANDLER(CXXNewExprProc) {
  if (isTrueProp(node, "is_new_array", false)) {
    return emitNewArrayExpr(node, w, src);
  }

  const auto is_global = isTrueProp(node, "is_global_new", false);
  const auto T = src.typeTable.at(getType(node));
  // FIXME: Support scalar type
  const auto pointeeT =
      llvm::cast<XcodeMl::Pointer>(T.get())->getPointee(src.typeTable);
  const auto NewTypeId = pointeeT->makeDeclaration(
      CXXCodeGen::makeVoidNode(), src.typeTable, src.nnsTable);
  /* Ref: [new.expr]/4
   * new int(*[10])();   // error
   * new (int(*[10])()); // OK
   * new int;            // OK
   * new (int);          // OK
   * new ((int));        // error
   */
  if (isTrueProp(node, "has_initializer", false)) {
    // The first element is initializer
    const auto init = findFirst(node, "clangStmt[1]", src.ctxt);
    // Remaining elements are placement arguments
    const auto placementArgs =
        createNodes(node, "clangStmt[position() > 1]", w, src);
    const auto placementArgSequence = placementArgs.empty()
        ? CXXCodeGen::makeVoidNode()
        : wrapWithParen(join(",", placementArgs));

    return (is_global ? makeTokenNode("::") : CXXCodeGen::makeVoidNode())
        + makeTokenNode("new") + placementArgSequence
        + wrapWithXcodeMlIdentity(NewTypeId)
        + wrapWithParen(join(",", w.walkChildren(init, src)));
  } else {
    const auto placementArgs = createNodes(node, "clangStmt", w, src);
    const auto placementArgSequence = placementArgs.empty()
        ? CXXCodeGen::makeVoidNode()
        : wrapWithParen(join(",", placementArgs));
    return (is_global ? makeTokenNode("::") : CXXCodeGen::makeVoidNode())
        + makeTokenNode("new") + placementArgSequence
        + wrapWithXcodeMlIdentity(NewTypeId);
  }
}

DEFINE_STMTHANDLER(CXXNullPtrLiteralExprProc) {
  return makeTokenNode("nullptr");
}

DEFINE_STMTHANDLER(CXXReinterpretCastExprProc) {
  const auto type = createNode(node, "clangTypeLoc", w, src);
  const auto expr = createNode(node, "clangStmt", w, src);
  return makeTokenNode("reinterpret_cast") + makeTokenNode("<") + type
      + makeTokenNode(">") + wrapWithParen(expr);
}

DEFINE_STMTHANDLER(CXXStaticCastExprProc) {
  const auto type = createNode(node, "clangTypeLoc", w, src);
  const auto expr = createNode(node, "clangStmt", w, src);
  return makeTokenNode("static_cast") + makeTokenNode("<") + type
      + makeTokenNode(">") + wrapWithParen(expr);
}

DEFINE_STMTHANDLER(CXXThrowExprProc) {
  if (auto throwExpr = findFirst(node, "clangStmt[1]", src.ctxt)) {
    return makeTokenNode("throw") + w.walk(throwExpr, src);
  } else {
    return makeTokenNode("throw");
  }
}

DEFINE_STMTHANDLER(CXXTryStmtProc) {
  const auto body = createNode(node, "clangStmt[1]", w, src);
  const auto catchClauses =
      createNodes(node, "clangStmt[@class='CXXCatchStmt']", w, src);
  return makeTokenNode("try") + body + insertNewLines(catchClauses);
}

DEFINE_STMTHANDLER(DeclRefExprProc) {
  const auto name = getQualifiedName(node, src);

  const auto TAL = findNodes(node, "TemplateArgumentLoc", src.ctxt);
  if(TAL.size() != 0){
    std::vector<CodeFragment> args;
    for(auto &&talNodes : TAL){
      const auto templArgNodes = findNodes(talNodes, "*", src.ctxt);
      for (auto &&argNode : templArgNodes) {
	args.push_back(w.walk(argNode, src));
      }
    }
    return name.toString(src.typeTable, src.nnsTable) + makeTokenNode("<")
        + join(",", args) + makeTokenNode(">");
  }

  return name.toString(src.typeTable, src.nnsTable);
}
DEFINE_STMTHANDLER(DependentScopeDeclRefExprProc) {
  const auto memberNode = findFirst(node, "clangDeclarationNameInfo[@class='Identifier']", src.ctxt);
  const auto nsnode = findFirst(node, "clangNestedNameSpecifier",
				src.ctxt);
  const auto member = makeTokenNode(getContent(memberNode));

  auto ns =  ClangNestedNameSpecHandler.walk(nsnode, src);

  return ns+member;
}

DEFINE_STMTHANDLER(DefaultStmtProc) {
  const auto body = createNode(node, "clangStmt", w, src);
  return makeTokenNode("default:") + body;
}

DEFINE_STMTHANDLER(DoStmtProc) {
  const auto body = createNode(node, "clangStmt[1]", w, src);
  const auto cond = createNode(node, "clangStmt[2]", w, src);
  return makeTokenNode("do") + makeCompoundStmt(body) + makeTokenNode("while")
      + wrapWithParen(cond);
}

DEFINE_STMTHANDLER(DeclStmtProc) {
  const auto declNodes = createNodes(node, "clangDecl", w, src);
  return join(";", declNodes);
}

DEFINE_STMTHANDLER(emitTokenAttrValue) {
  const auto token = getProp(node, "token");
  return makeTokenNode(token);
}

DEFINE_STMTHANDLER(emitIntegerLiteral) {
  const auto token = getProp(node, "token");
  if (token.size() > 0) {
    return makeTokenNode(token);
  } else {
    // overloaded postIncrExpr & postDecrExpr has a dummy argument:
    //  <clangStmt class="IntegerLiteral" token="" decimalNotation="0"/>
    // we must handle this special case.
    return makeTokenNode(getProp(node, "decimalNotation"));
  }
}

DEFINE_STMTHANDLER(CXXTemporaryObjectExprProc) {
  const auto resultT = src.typeTable.at(getType(node));
  const auto name = makeDecl(
      resultT, CXXCodeGen::makeVoidNode(), src.typeTable, src.nnsTable);
  const auto args = createNodes(node, "clangStmt", w, src);
  return wrapWithXcodeMlIdentity(name) + wrapWithParen(join(",", args));
}

DEFINE_STMTHANDLER(CXXOperatorCallExprProc) {
  const auto callee = createNode(node, "clangStmt[1]", w, src);
  if (isTrueProp(node, "is_member_function", false)) {
    const auto lhs = createNode(node, "clangStmt[2]", w, src);
    const auto args = createNodes(node, "clangStmt[position() > 2]", w, src);
    return lhs + makeTokenNode(".") + callee + wrapWithParen(join(",", args));
  } else {
    const auto args = createNodes(node, "clangStmt[position() > 1]", w, src);
    return callee + wrapWithParen(join(",", args));
  }
}

DEFINE_STMTHANDLER(ForStmtProc) {
  /*
   * {
   *   _init-statement(s)_
   *   for ( ; _condition-opt_ ; _iteration-expression-opt_ ) {
   *     _body-statement_
   *   }
   * }
   */
  const auto initNode =
      findFirst(node, "clangStmt[@for_stmt_kind='init']", src.ctxt);
  const auto init =
      (initNode ? w.walk(initNode, src) : CXXCodeGen::makeVoidNode())
      + makeTokenNode(";");
  const auto cond =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='cond']", w, src);
  const auto iter =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='iter']", w, src);
  const auto body =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='body']", w, src);
  const auto head = makeTokenNode("for")
      + wrapWithParen(makeTokenNode(";") + cond + makeTokenNode(";") + iter);
  return wrapWithBrace(init + head + makeCompoundStmt(body));
}

DEFINE_STMTHANDLER(GotoStmtProc) {
  const auto label = getProp(node, "label_name");
  return makeTokenNode("goto") + makeTokenNode(label);
}

DEFINE_STMTHANDLER(IfStmtProc) {
  const auto cond = createNode(node, "clangStmt[1]", w, src);
  const auto then = createNode(node, "clangStmt[2]", w, src);
  const auto head =
      makeTokenNode("if") + wrapWithParen(cond) + makeCompoundStmt(then);

  if (const auto elseNode = findFirst(node, "clangStmt[3]", src.ctxt)) {
    const auto Else = w.walk(elseNode, src);
    return head + makeTokenNode("else") + makeCompoundStmt(Else);
  }
  return head;
}

DEFINE_STMTHANDLER(InitListExprProc) {
  const auto members = createNodes(node, "clangStmt", w, src);
  return wrapWithBrace(join(",", members));
}

DEFINE_STMTHANDLER(LabelStmtProc) {
  const auto label = getProp(node, "label_name");
  const auto body = createNode(node, "clangStmt[1]", w, src);
  return makeTokenNode(label) + makeTokenNode(":") + body;
}

DEFINE_STMTHANDLER(CXXDependentScopeMemberExprProc){
  bool isArrow = false;
  auto expr = CXXCodeGen::makeVoidNode();
  if(findFirst(node, "clangStmt", src.ctxt)){
    expr = expr +  createNode(node, "clangStmt", w, src);
    isArrow = isTrueProp(node, "is_arrow", false);

  }else {
    const auto nsnode = findFirst(node, "clangNestedNameSpecifier",
				  src.ctxt);
    expr =  expr + ClangNestedNameSpecHandler.walk(nsnode, src);
  }
    const auto memberNode = findFirst(node, "clangDeclarationNameInfo[@class='Identifier']", src.ctxt);
    const auto member = makeTokenNode(getContent(memberNode));

    return expr + makeTokenNode(isArrow ? "->" : ".") + member;
}

DEFINE_STMTHANDLER(MemberExprProc) {
  const auto expr = createNode(node, "clangStmt", w, src);
  const auto member =
      getQualifiedName(node, src).toString(src.typeTable, src.nnsTable);
  const auto isArrow = isTrueProp(node, "is_arrow", false);
  return expr + makeTokenNode(isArrow ? "->" : ".") + member;
}
DEFINE_STMTHANDLER(PackExpansionProc) {
  const auto expr = makeInnerNode(ProgramBuilder.walkChildren(node, src));
  return expr + makeTokenNode("...");
}

DEFINE_STMTHANDLER(ReturnStmtProc) {
  if (const auto exprNode = findFirst(node, "clangStmt", src.ctxt)) {
    const auto expr = w.walk(exprNode, src);
    return makeTokenNode("return") + expr;
  }
  return makeTokenNode("return");
}

DEFINE_STMTHANDLER(StringLiteralProc) {
  const auto string = makeTokenNode(getProp(node, "stringLiteral"));
  return makeTokenNode("\"") + string + makeTokenNode("\"");
}

DEFINE_STMTHANDLER(SwitchStmtProc) {
  const auto exprNode = findFirst(node, "clangStmt[1]", src.ctxt);
  const auto expr = w.walk(exprNode, src);

  const auto bodyNode = findFirst(node, "clangStmt[2]", src.ctxt);
  const auto body = w.walk(bodyNode, src);

  return makeTokenNode("switch") + wrapWithParen(expr) + body;
}

DEFINE_STMTHANDLER(ThisExprProc) {
  return makeTokenNode("this");
}

DEFINE_STMTHANDLER(UnaryOperatorProc) {
  const auto expr = createNode(node, "clangStmt", w, src);

  const auto opName = getProp(node, "unaryOpName");
  const auto opSpelling = XcodeMl::OperatorNameToSpelling(opName);
  if (!opSpelling.hasValue()) {
    std::cerr << "Unknown operator name: '" << opName << "'" << std::endl;
    std::abort();
  }
  const auto op = makeTokenNode(*opSpelling);
  const auto postfix = std::equal(opName.begin(), opName.end(), "postDecrExpr")
      || std::equal(opName.begin(), opName.end(), "postIncrExpr");
  return wrapWithParen(postfix ? expr + op : op + expr);
}

DEFINE_STMTHANDLER(WhileStmtProc) {
  const auto cond = createNode(node, "clangStmt[1]", w, src);
  const auto body = createNode(node, "clangStmt[2]", w, src);
  return makeTokenNode("while") + wrapWithParen(cond) + makeCompoundStmt(body);
}

} // namespace

const ClangStmtHandlerType ClangStmtHandler("class",
    cxxgen::makeInnerNode,
    callCodeBuilder,
    {
        std::make_tuple("ArraySubscriptExpr", ArraySubscriptExprProc),
        std::make_tuple(
            "BinaryConditionalOperator", BinaryConditionalOperatorProc),
        std::make_tuple("BinaryOperator", BinaryOperatorProc),
        std::make_tuple("BreakStmt", BreakStmtProc),
        std::make_tuple("CallExpr", callExprProc),
        std::make_tuple("CaseStmt", CaseStmtProc),
        std::make_tuple("ConditionalOperator", ConditionalOperatorProc),
        std::make_tuple("CharacterLiteral", emitTokenAttrValue),
        std::make_tuple("CompoundAssignOperator", BinaryOperatorProc),
        std::make_tuple("CompoundStmt", CompoundStmtProc),
        std::make_tuple("ContinueStmt", ContinueStmtProc),
        std::make_tuple("CStyleCastExpr", CStyleCastExprProc),
        std::make_tuple("CXXBoolLiteralExpr", CXXBoolLiteralExprProc),
        std::make_tuple("CXXCatchStmt", CXXCatchStmtProc),
        std::make_tuple("CXXConstCastExpr", CXXConstCastExprProc),
        std::make_tuple("CXXConstructExpr", CXXCtorExprProc),
        std::make_tuple("CXXDeleteExpr", CXXDeleteExprProc),
        std::make_tuple("CXXDynamicCastExpr", CXXDynamicCastExprProc),
        std::make_tuple("CXXFunctionalCastExpr", CStyleCastExprProc),
        std::make_tuple("CXXMemberCallExpr", callExprProc),
        std::make_tuple("CXXNewExpr", CXXNewExprProc),
        std::make_tuple("CXXNullPtrLiteralExpr", CXXNullPtrLiteralExprProc),
        std::make_tuple("CXXOperatorCallExpr", CXXOperatorCallExprProc),
        std::make_tuple("CXXReinterpretCastExpr", CXXReinterpretCastExprProc),
        std::make_tuple("CXXStaticCastExpr", CXXStaticCastExprProc),
        std::make_tuple("CXXTemporaryObjectExpr", CXXTemporaryObjectExprProc),
        std::make_tuple("CXXThrowExpr", CXXThrowExprProc),
        std::make_tuple("CXXTryStmt", CXXTryStmtProc),
        std::make_tuple("CXXThisExpr", ThisExprProc),
        std::make_tuple("CXXUnresolvedConstructExpr", CXXCtorExprProc),
        std::make_tuple("DeclStmt", DeclStmtProc),
        std::make_tuple("DeclRefExpr", DeclRefExprProc),
        std::make_tuple("DefaultStmt", DefaultStmtProc),
        std::make_tuple("DoStmt", DoStmtProc),
        std::make_tuple("FloatingLiteral", emitTokenAttrValue),
        std::make_tuple("ForStmt", ForStmtProc),
        std::make_tuple("GotoStmt", GotoStmtProc),
        std::make_tuple("IfStmt", IfStmtProc),
        std::make_tuple("InitListExpr", InitListExprProc),
        std::make_tuple("IntegerLiteral", emitIntegerLiteral),
        std::make_tuple("LabelStmt", LabelStmtProc),
        std::make_tuple("MemberExpr", MemberExprProc),
	std::make_tuple("PackExpansionExpr", PackExpansionProc),
	std::make_tuple("CXXDependentScopeMemberExpr", CXXDependentScopeMemberExprProc),
	std::make_tuple("DependentScopeDeclRefExpr", DependentScopeDeclRefExprProc),
        std::make_tuple("ReturnStmt", ReturnStmtProc),
        std::make_tuple("StringLiteral", StringLiteralProc),
        std::make_tuple("SwitchStmt", SwitchStmtProc),
        std::make_tuple("UnaryOperator", UnaryOperatorProc),
        std::make_tuple("WhileStmt", WhileStmtProc),
    });
