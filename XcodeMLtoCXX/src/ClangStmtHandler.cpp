#include <algorithm>
#include <functional>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
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
#include "XcodeMlEnvironment.h"
#include "XcodeMlOperator.h"
#include "SourceInfo.h"
#include "TypeAnalyzer.h"
#include "NnsAnalyzer.h"
#include "CodeBuilder.h"
#include "ClangStmtHandler.h"
#include "XcodeMlUtil.h"

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
              << "not found: '" << xpath << "'" << std::endl;
    std::abort();
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

DEFINE_STMTHANDLER(callCodeBuilder) {
  return makeInnerNode(ProgramBuilder.walkChildren(node, src));
}

DEFINE_STMTHANDLER(BinaryOperatorProc) {
  const auto lhsNode = findFirst(node, "clangStmt[1]", src.ctxt);
  const auto lhs = w.walk(lhsNode, src);
  const auto rhsNode = findFirst(node, "clangStmt[2]", src.ctxt);
  const auto rhs = w.walk(rhsNode, src);

  const auto opName = getProp(node, "binOpName");
  const auto opSpelling = XcodeMl::OperatorNameToSpelling(opName);
  if (!opSpelling.hasValue()) {
    std::cerr << "Unknown operator name: '" << opName << "'" << std::endl;
    std::abort();
  }
  return lhs + makeTokenNode(*opSpelling) + rhs;
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

  return func + wrapWithParen(join(",", args));
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
  return cond + makeTokenNode("?") + yes + makeTokenNode(":") + no;
}

DEFINE_STMTHANDLER(CXXCatchStmtProc) {
  const auto body = createNode(node, "clangStmt", w, src);
  if (const auto declNode = findFirst(node, "clangDecl", src.ctxt)) {
    const auto var = w.walk(declNode, src);
    return makeTokenNode("catch") + wrapWithParen(var) + body;
  } else {
    return makeTokenNode("catch(...)") + body;
  }
}

DEFINE_STMTHANDLER(CXXCtorExprProc) {
  return makeTokenNode("(") + cxxgen::join(", ", w.walkChildren(node, src))
      + makeTokenNode(")");
}

DEFINE_STMTHANDLER(CXXDeleteExprProc) {
  const auto allocated = findFirst(node, "*", src.ctxt);
  return makeTokenNode("delete") + w.walk(allocated, src);
}

DEFINE_STMTHANDLER(CXXNewExprProc) {
  const auto T = src.typeTable.at(getType(node));
  // FIXME: Support scalar type
  const auto pointeeT =
      llvm::cast<XcodeMl::Pointer>(T.get())->getPointee(src.typeTable);
  const auto NewTypeId =
      pointeeT->makeDeclaration(CXXCodeGen::makeVoidNode(), src.typeTable);
  /* Ref: [new.expr]/4
   * new int(*[10])();   // error
   * new (int(*[10])()); // OK
   * new int;            // OK
   * new (int);          // OK
   * new ((int));        // error
   */
  const auto init = findFirst(node, "clangStmt", src.ctxt);

  return makeTokenNode("new")
      + (hasParen(pointeeT, src.typeTable) ? wrapWithParen(NewTypeId)
                                           : NewTypeId)
      + (init ? wrapWithParen(join(",", w.walkChildren(init, src)))
              : CXXCodeGen::makeVoidNode());
}

DEFINE_STMTHANDLER(CXXTryStmtProc) {
  const auto body = createNode(node, "clangStmt[1]", w, src);
  const auto catchClauses =
      createNodes(node, "clangStmt[@class='CXXCatchStmt']", w, src);
  return makeTokenNode("try") + body + insertNewLines(catchClauses);
}

DEFINE_STMTHANDLER(DeclRefExprProc) {
  const auto name = getQualifiedName(node, src);

  if (const auto TAL = findFirst(node, "TemplateArgumentLoc", src.ctxt)) {
    const auto templArgNodes = findNodes(TAL, "*", src.ctxt);
    std::vector<CodeFragment> args;
    for (auto &&argNode : templArgNodes) {
      args.push_back(w.walk(argNode, src));
    }
    return name.toString(src.typeTable, src.nnsTable) + makeTokenNode("<")
        + join(",", args) + makeTokenNode(">");
  }

  return name.toString(src.typeTable, src.nnsTable);
}

DEFINE_STMTHANDLER(DoStmtProc) {
  const auto body = createNode(node, "clangStmt[1]", w, src);
  const auto cond = createNode(node, "clangStmt[2]", w, src);
  return makeTokenNode("do") + body + makeTokenNode("while")
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

DEFINE_STMTHANDLER(CXXTemporaryObjectExprProc) {
  const auto resultT = src.typeTable.at(getType(node));
  const auto name = llvm::cast<XcodeMl::ClassType>(resultT.get())->name();
  assert(name.hasValue());
  auto children = findNodes(node, "*[position() > 1]", src.ctxt);
  // ignore first child, which represents the result (class) type of
  // the clang::CXXTemporaryObjectExpr
  std::vector<CodeFragment> args;
  for (auto child : children) {
    args.push_back(w.walk(child, src));
  }
  return *name + makeTokenNode("(") + join(",", args) + makeTokenNode(")");
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
  const auto initNode =
      findFirst(node, "clangStmt[@for_stmt_kind='init']", src.ctxt);
  const auto init =
      initNode ? w.walk(initNode, src) : CXXCodeGen::makeVoidNode();
  const auto cond =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='cond']", w, src);
  const auto iter =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='iter']", w, src);
  const auto body =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='body']", w, src);
  const auto head =
      init + makeTokenNode(";") + cond + makeTokenNode(";") + iter;
  return makeTokenNode("for") + wrapWithParen(head) + body;
}

DEFINE_STMTHANDLER(IfStmtProc) {
  const auto cond = createNode(node, "clangStmt[1]", w, src);
  const auto body = createNode(node, "clangStmt[2]", w, src);
  return makeTokenNode("if") + wrapWithParen(cond) + body;
}

DEFINE_STMTHANDLER(InitListExprProc) {
  const auto members = createNodes(node, "clangStmt", w, src);
  return wrapWithBrace(join(",", members));
}

DEFINE_STMTHANDLER(MemberExprProc) {
  const auto expr = createNode(node, "clangStmt", w, src);
  const auto member =
      getQualifiedName(node, src).toString(src.typeTable, src.nnsTable);
  const auto isArrow = isTrueProp(node, "is_arrow", false);
  return expr + makeTokenNode(isArrow ? "->" : ".") + member;
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
  return makeTokenNode("while") + wrapWithParen(cond) + body;
}

} // namespace

const ClangStmtHandlerType ClangStmtHandler("class",
    cxxgen::makeInnerNode,
    callCodeBuilder,
    {
        std::make_tuple("BinaryOperator", BinaryOperatorProc),
        std::make_tuple("BreakStmt", BreakStmtProc),
        std::make_tuple("CallExpr", callExprProc),
        std::make_tuple("CaseStmt", CaseStmtProc),
        std::make_tuple("ConditionalOperator", ConditionalOperatorProc),
        std::make_tuple("CharacterLiteral", emitTokenAttrValue),
        std::make_tuple("CompoundAssignOperator", BinaryOperatorProc),
        std::make_tuple("CompoundStmt", CompoundStmtProc),
        std::make_tuple("CXXCatchStmt", CXXCatchStmtProc),
        std::make_tuple("CXXConstructExpr", CXXCtorExprProc),
        std::make_tuple("CXXDeleteExpr", CXXDeleteExprProc),
        std::make_tuple("CXXMemberCallExpr", callExprProc),
        std::make_tuple("CXXNewExpr", CXXNewExprProc),
        std::make_tuple("CXXOperatorCallExpr", CXXOperatorCallExprProc),
        std::make_tuple("CXXTemporaryObjectExpr", CXXTemporaryObjectExprProc),
        std::make_tuple("CXXTryStmt", CXXTryStmtProc),
        std::make_tuple("CXXThisExpr", ThisExprProc),
        std::make_tuple("DeclStmt", DeclStmtProc),
        std::make_tuple("DeclRefExpr", DeclRefExprProc),
        std::make_tuple("DoStmt", DoStmtProc),
        std::make_tuple("FloatingLiteral", emitTokenAttrValue),
        std::make_tuple("ForStmt", ForStmtProc),
        std::make_tuple("IfStmt", IfStmtProc),
        std::make_tuple("InitListExpr", InitListExprProc),
        std::make_tuple("IntegerLiteral", emitTokenAttrValue),
        std::make_tuple("MemberExpr", MemberExprProc),
        std::make_tuple("ReturnStmt", ReturnStmtProc),
        std::make_tuple("StringLiteral", StringLiteralProc),
        std::make_tuple("SwitchStmt", SwitchStmtProc),
        std::make_tuple("UnaryOperator", UnaryOperatorProc),
        std::make_tuple("WhileStmt", WhileStmtProc),
    });
