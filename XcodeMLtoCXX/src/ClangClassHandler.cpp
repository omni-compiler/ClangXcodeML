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
#include "ClangClassHandler.h"
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

} // namespace

#define DECLHANDLER_ARGS                                                      \
  xmlNodePtr node __attribute__((unused)),                                    \
      const CodeBuilder &w __attribute__((unused)),                           \
      SourceInfo &src __attribute__((unused))

#define DEFINE_DECLHANDLER(name) XcodeMl::CodeFragment name(DECLHANDLER_ARGS)

DEFINE_DECLHANDLER(callCodeBuilder) {
  return makeInnerNode(ProgramBuilder.walkChildren(node, src));
}

DEFINE_DECLHANDLER(emitInlineMemberFunction) {
  if (isTrueProp(node, "is_implicit", 0)) {
    return CXXCodeGen::makeVoidNode();
  }

  auto acc = CXXCodeGen::makeVoidNode();
  if (isTrueProp(node, "is_virtual", false)) {
    acc = acc + makeTokenNode("virtual");
  }
  if (isTrueProp(node, "is_static", false)) {
    acc = acc + makeTokenNode("static");
  }
  const auto paramNames = getParamNames(node, src);
  acc = acc + makeFunctionDeclHead(node, paramNames, src);

  if (const auto ctorInitList =
          findFirst(node, "constructorInitializerList", src.ctxt)) {
    acc = acc + ProgramBuilder.walk(ctorInitList, src);
  }

  if (const auto bodyNode = findFirst(node, "clangStmt", src.ctxt)) {
    const auto body = ProgramBuilder.walk(bodyNode, src);
    return acc + body;
  } else {
    return acc + makeTokenNode(";");
  }
  return acc;
}

DEFINE_DECLHANDLER(FieldDeclProc) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto name = getUnqualIdFromNameNode(nameNode)->toString(src.typeTable);

  const auto dtident = getType(node);
  const auto T = src.typeTable.at(dtident);

  return makeDecl(T, name, src.typeTable) + makeTokenNode(";");
}

DEFINE_DECLHANDLER(emitTokenAttrValue);
DEFINE_DECLHANDLER(CXXRecordProc);
DEFINE_DECLHANDLER(VarProc);

const ClangClassHandler ClassDefinitionDeclHandler("class",
    CXXCodeGen::makeInnerNode,
    callCodeBuilder,
    {
        std::make_tuple("CharacterLiteral", emitTokenAttrValue),
        std::make_tuple("CXXMethod", emitInlineMemberFunction),
        std::make_tuple("CXXConstructor", emitInlineMemberFunction),
        std::make_tuple("CXXDestructor", emitInlineMemberFunction),
        std::make_tuple("CXXRecord", CXXRecordProc),
        std::make_tuple("FloatingLiteral", emitTokenAttrValue),
        std::make_tuple("Field", FieldDeclProc),
        std::make_tuple("IntegerLiteral", emitTokenAttrValue),
        std::make_tuple("Var", VarProc),
    });

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
    stmts.push_back(w.walk(stmtNode, src) + makeTokenNode(";"));
  }
  return wrapWithBrace(insertNewLines(stmts));
}

DEFINE_STMTHANDLER(ConditionalOperatorProc) {
  const auto cond = createNode(node, "clangStmt[1]", w, src);
  const auto yes = createNode(node, "clangStmt[2]", w, src);
  const auto no = createNode(node, "clangStmt[3]", w, src);
  return cond + makeTokenNode("?") + yes + makeTokenNode(":") + no;
}

namespace {

CodeFragment
makeTemplateHead(xmlNodePtr node, const CodeBuilder &w, SourceInfo &src) {
  const auto paramNodes =
      findNodes(node, "clangDecl[@class='TemplateTypeParm']", src.ctxt);
  std::vector<CXXCodeGen::StringTreeRef> params;
  for (auto &&paramNode : paramNodes) {
    params.push_back(w.walk(paramNode, src));
  }
  return makeTokenNode("template") + makeTokenNode("<") + join(",", params)
      + makeTokenNode(">");
}

} // namespace

DEFINE_DECLHANDLER(ClassTemplateProc) {
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandEnvironment(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsMap(src.nnsTable, nnsTableNode, src.ctxt);
  }
  const auto bodyNode = findFirst(node, "clangDecl[@class='CXXRecord']", src.ctxt);

  const auto head = makeTemplateHead(node, w, src);
  const auto body = w.walk(bodyNode, src);
  return head + body;
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
      + wrapWithParen(cond) + makeTokenNode(";");
}

DEFINE_STMTHANDLER(DeclStmtProc) {
  const auto declNodes = createNodes(node, "clangDecl", w, src);
  return insertNewLines(declNodes);
}

DEFINE_STMTHANDLER(emitTokenAttrValue) {
  const auto token = getProp(node, "token");
  return makeTokenNode(token);
}

DEFINE_DECLHANDLER(FunctionTemplateProc) {
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandEnvironment(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsMap(src.nnsTable, nnsTableNode, src.ctxt);
  }
  const auto paramNodes =
      findNodes(node, "clangDecl[@class='TemplateTypeParm']", src.ctxt);
  const auto body = findFirst(node, "clangDecl", src.ctxt);

  std::vector<CXXCodeGen::StringTreeRef> params;
  for (auto &&paramNode : paramNodes) {
    params.push_back(w.walk(paramNode, src));
  }

  return makeTokenNode("template") + makeTokenNode("<") + join(",", params)
      + makeTokenNode(">") + w.walk(body, src);
}

DEFINE_DECLHANDLER(TemplateTypeParmProc) {
  const auto name = getQualifiedName(node, src);
  const auto nameSpelling = name.toString(src.typeTable, src.nnsTable);

  const auto dtident = getType(node);
  auto T = src.typeTable.at(dtident);
  auto TTPT = llvm::cast<XcodeMl::TemplateTypeParm>(T.get());
  assert(TTPT);
  TTPT->setSpelling(nameSpelling);

  return makeTokenNode("typename") + nameSpelling;
}

XcodeMl::CodeFragment
makeBases(const XcodeMl::ClassType &T, SourceInfo &src) {
  using namespace XcodeMl;
  const auto bases = T.getBases();
  std::vector<CodeFragment> decls;
  std::transform(bases.begin(),
      bases.end(),
      std::back_inserter(decls),
      [&src](ClassType::BaseClass base) {
        const auto T = src.typeTable.at(std::get<1>(base));
        const auto classT = llvm::cast<ClassType>(T.get());
        assert(classT);
        assert(classT->name().hasValue());
        return makeTokenNode(std::get<0>(base))
            + makeTokenNode(std::get<2>(base) ? "virtual" : "")
            + *(classT->name());
      });
  return decls.empty() ? cxxgen::makeVoidNode()
                       : makeTokenNode(":") + cxxgen::join(",", decls);
}

CodeFragment
emitClassDefinition(xmlNodePtr node,
    const CodeBuilder &w,
    SourceInfo &src,
    const XcodeMl::ClassType &classType) {
  if (isTrueProp(node, "is_implicit", false)) {
    return cxxgen::makeVoidNode();
  }

  const auto memberNodes = findNodes(node, "clangDecl", src.ctxt);
  std::vector<XcodeMl::CodeFragment> decls;
  for (auto &&memberNode : memberNodes) {
    if (isTrueProp(memberNode, "is_implicit", false)) {
      continue;
    }
    /* Traverse `memberNode` regardless of whether `CodeBuilder` prints it. */
    const auto decl = w.walk(memberNode, src);

    const auto accessProp = getPropOrNull(memberNode, "access");
    if (accessProp.hasValue()) {
      const auto access = *accessProp;
      decls.push_back(makeTokenNode(access) + makeTokenNode(":") + decl);
    } else {
      decls.push_back(makeTokenNode(
          "\n/* Ignored a member with no access specifier */\n"));
    }
  }

  const auto classKey = makeTokenNode(getClassKey(classType.classKind()));
  const auto name = classType.isClassTemplateSpecialization()
      ? classType.getAsTemplateId(src.typeTable).getValue()
      : classType.name().getValue();

  return classKey + name + makeBases(classType, src) + makeTokenNode("{")
      + separateByBlankLines(decls) + makeTokenNode("}") + makeTokenNode(";")
      + cxxgen::makeNewLineNode();
}

void
setClassName(XcodeMl::ClassType &classType, xmlNodePtr node, SourceInfo &src) {
  if (classType.name().hasValue()) {
    return;
  }
  /* `classType` is unnamed.
   * Unnamed classes are problematic, so give a name to `classType`
   * such as `__xcodeml_1`.
   */
  classType.setName(src.getUniqueName());
}

DEFINE_DECLHANDLER(CXXRecordProc) {
  if (isTrueProp(node, "is_implicit", false)) {
    return cxxgen::makeVoidNode();
  }

  const auto T = src.typeTable.at(getType(node));
  auto classT = llvm::dyn_cast<XcodeMl::ClassType>(T.get());
  assert(classT);

  setClassName(*classT, node, src);
  const auto nameSpelling = *(classT->name()); // now class name must exist

  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    return emitClassDefinition(node, ClassDefinitionBuilder, src, *classT);
  }

  /* forward declaration */
  const auto classKey = getClassKey(classT->classKind());
  return makeTokenNode(classKey) + nameSpelling + makeTokenNode(";");
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
  const auto init = initNode ? w.walk(initNode, src) : makeTokenNode(";");
  const auto cond =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='cond']", w, src);
  const auto iter =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='iter']", w, src);
  const auto body =
      createNodeOrNull(node, "clangStmt[@for_stmt_kind='body']", w, src);
  return makeTokenNode("for")
      + wrapWithParen(init + cond + makeTokenNode(";") + iter) + body;
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
    return makeTokenNode("return") + expr + makeTokenNode(";");
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

const ClangClassHandler ClangStmtHandler("class",
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
        std::make_tuple("CXXConstructExpr", CXXCtorExprProc),
        std::make_tuple("CXXDeleteExpr", CXXDeleteExprProc),
        std::make_tuple("CXXMemberCallExpr", callExprProc),
        std::make_tuple("CXXNewExpr", CXXNewExprProc),
        std::make_tuple("CXXOperatorCallExpr", CXXOperatorCallExprProc),
        std::make_tuple("CXXTemporaryObjectExpr", CXXTemporaryObjectExprProc),
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

DEFINE_DECLHANDLER(ClassTemplateSpecializationProc) {
  const auto T = src.typeTable.at(getType(node));
  const auto classT = llvm::dyn_cast<XcodeMl::ClassType>(T.get());
  assert(classT && classT->name().hasValue());
  const auto nameSpelling = classT->name().getValue();

  const auto head =
      makeTokenNode("template") + makeTokenNode("<") + makeTokenNode(">");

  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    return head
        + emitClassDefinition(node, ClassDefinitionBuilder, src, *classT);
  }

  /* forward declaration */
  const auto classKey = getClassKey(classT->classKind());
  return head + makeTokenNode(classKey) + nameSpelling + makeTokenNode(";");
}

DEFINE_DECLHANDLER(ClassTemplatePartialSpecializationProc) {
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandEnvironment(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsMap(src.nnsTable, nnsTableNode, src.ctxt);
  }

  const auto T = src.typeTable.at(getType(node));
  const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
  const auto head = makeTemplateHead(node, w, src);
  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    const auto def =
        emitClassDefinition(node, ClassDefinitionBuilder, src, *classT);
    return head + def;
  }
  /* forward declaration */
  const auto classKey = getClassKey(classT->classKind());
  const auto nameSpelling = classT->name().getValue();
  return head + makeTokenNode(classKey) + nameSpelling + makeTokenNode(";");
}

DEFINE_DECLHANDLER(FriendDeclProc) {
  if (auto TL = findFirst(node, "clangTypeLoc", src.ctxt)) {
    /* friend class declaration */
    const auto dtident = getType(TL);
    const auto T = src.typeTable.at(dtident);
    return makeTokenNode("friend")
        + makeDecl(T, cxxgen::makeVoidNode(), src.typeTable)
        + makeTokenNode(";");
  }
  return makeTokenNode("friend") + callCodeBuilder(node, w, src);
}

DEFINE_DECLHANDLER(FunctionProc) {
  if (isTrueProp(node, "is_implicit", 0)) {
    return cxxgen::makeVoidNode();
  }
  const auto type = getProp(node, "xcodemlType");
  const auto paramNames = getParamNames(node, src);
  auto acc = makeFunctionDeclHead(node, paramNames, src, true);

  if (const auto ctorInitList =
          findFirst(node, "constructorInitializerList", src.ctxt)) {
    acc = acc + w.walk(ctorInitList, src);
  }

  if (const auto bodyNode = findFirst(node, "clangStmt", src.ctxt)) {
    const auto body = w.walk(bodyNode, src);
    acc = acc + body;
  } else {
    acc = acc + makeTokenNode(";");
  }

  return wrapWithLangLink(acc, node, src);
}

void
setStructName(XcodeMl::Struct &s, xmlNodePtr node, SourceInfo &src) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  if (!nameNode || isEmpty(nameNode)) {
    s.setTagName(makeTokenNode(src.getUniqueName()));
    return;
  }
  const auto name = getUnqualIdFromNameNode(nameNode);
  const auto nameSpelling = name->toString(src.typeTable);
  s.setTagName(nameSpelling);
}

DEFINE_DECLHANDLER(RecordProc) {
  if (isTrueProp(node, "is_implicit", false)) {
    return cxxgen::makeVoidNode();
  }
  const auto T = src.typeTable.at(getType(node));
  auto structT = llvm::dyn_cast<XcodeMl::Struct>(T.get());
  assert(structT);
  setStructName(*structT, node, src);
  const auto tagName = structT->tagName();

  const auto decls = createNodes(node, "clangDecl", w, src);
  return makeTokenNode("struct") + tagName
      + wrapWithBrace(insertNewLines(decls)) + makeTokenNode(";");
}

DEFINE_DECLHANDLER(TranslationUnitProc) {
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandEnvironment(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsMap(src.nnsTable, nnsTableNode, src.ctxt);
  }
  const auto declNodes = findNodes(node, "clangDecl", src.ctxt);
  std::vector<CXXCodeGen::StringTreeRef> decls;
  for (auto &&declNode : declNodes) {
    decls.push_back(w.walk(declNode, src));
  }
  return separateByBlankLines(decls);
}

DEFINE_DECLHANDLER(TypedefProc) {
  if (isTrueProp(node, "is_implicit", 0)) {
    return cxxgen::makeVoidNode();
  }
  const auto dtident = getProp(node, "xcodemlTypedefType");
  const auto T = src.typeTable.at(dtident);

  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto typedefName =
      getUnqualIdFromNameNode(nameNode)->toString(src.typeTable);

  return makeTokenNode("typedef") + makeDecl(T, typedefName, src.typeTable)
      + makeTokenNode(";");
}

CodeFragment
makeSpecifier(xmlNodePtr node) {
  const std::vector<std::tuple<std::string, std::string>> specifiers = {
      std::make_tuple("is_extern", "extern"),
      std::make_tuple("is_register", "register"),
      std::make_tuple("is_static", "static"),
      std::make_tuple("is_static_data_member", "static"),
      std::make_tuple("is_thread_local", "thread_local"),
  };
  auto code = CXXCodeGen::makeVoidNode();
  for (auto &&tuple : specifiers) {
    std::string attr, specifier;
    std::tie(attr, specifier) = tuple;
    if (isTrueProp(node, attr.c_str(), false)) {
      code = code + makeTokenNode(specifier);
    }
  }
  return code;
}

DEFINE_DECLHANDLER(VarProc) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto name = getUnqualIdFromNameNode(nameNode)->toString(src.typeTable);
  const auto dtident = getProp(node, "xcodemlType");
  const auto T = src.typeTable.at(dtident);

  const auto decl = makeSpecifier(node) + makeDecl(T, name, src.typeTable);
  const auto initializerNode = findFirst(node, "clangStmt", src.ctxt);
  if (!initializerNode) {
    // does not have initalizer: `int x;`
    return makeDecl(T, name, src.typeTable) + makeTokenNode(";");
  }
  const auto astClass = getProp(initializerNode, "class");
  if (std::equal(astClass.begin(), astClass.end(), "CXXConstructExpr")) {
    // has initalizer and the variable is of class type
    const auto init = declareClassTypeInit(w, initializerNode, src);
    return wrapWithLangLink(decl + init + makeTokenNode(";"), node, src);
  }
  const auto init = w.walk(initializerNode, src);
  return decl + makeTokenNode("=") + init + makeTokenNode(";");
}

const ClangClassHandler ClangDeclHandler("class",
    cxxgen::makeInnerNode,
    callCodeBuilder,
    {
        std::make_tuple("ClassTemplate", ClassTemplateProc),
        std::make_tuple(
            "ClassTemplateSpecialization", ClassTemplateSpecializationProc),
        std::make_tuple("ClassTemplatePartialSpecialization",
            ClassTemplatePartialSpecializationProc),
        std::make_tuple("CXXConstructor", FunctionProc),
        std::make_tuple("CXXMethod", FunctionProc),
        std::make_tuple("CXXRecord", CXXRecordProc),
        std::make_tuple("Field", FieldDeclProc),
        std::make_tuple("Friend", FriendDeclProc),
        std::make_tuple("Function", FunctionProc),
        std::make_tuple("FunctionTemplate", FunctionTemplateProc),
        std::make_tuple("Record", RecordProc),
        std::make_tuple("TemplateTypeParm", TemplateTypeParmProc),
        std::make_tuple("TranslationUnit", TranslationUnitProc),
        std::make_tuple("Typedef", TypedefProc),
        std::make_tuple("Var", VarProc),
    });
