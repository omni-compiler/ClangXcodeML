#include <functional>
#include <sstream>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "Stream.h"
#include "StringTree.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "AttrProc.h"
#include "Symbol.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SourceInfo.h"
#include "CodeBuilder.h"
#include "ClangClassHandler.h"

namespace cxxgen = CXXCodeGen;

#define CCH_ARGS xmlNodePtr node __attribute__((unused)), \
                 const CodeBuilder& w __attribute__((unused)), \
                 SourceInfo& src __attribute__((unused))

#define DEFINE_CCH(name) static XcodeMl::CodeFragment name(CCH_ARGS)

using cxxgen::makeInnerNode;
using cxxgen::makeTokenNode;
using XcodeMl::CodeFragment;

DEFINE_CCH(callCodeBuilder) {
  return makeInnerNode(w.walkChildren(node, src));
}

DEFINE_CCH(callExprProc) {
  return (w["functionCall"])(w, node, src);
}

DEFINE_CCH(CXXMemberCallExprProc) {
  auto funcNode = findFirst(node, "*[1]", src.ctxt);
  const auto func = w.walk(funcNode, src);
  const auto argNodes = findNodes(node, "*[position() > 1]", src.ctxt);
  std::vector<CodeFragment> args;
  for (auto a : argNodes) {
    args.push_back(w.walk(a, src));
  }
  return func +
    makeTokenNode("(") +
    cxxgen::join(", ", args) +
    makeTokenNode(")");
}

DEFINE_CCH(CXXTemporaryObjectExprProc) {
  const auto resultT = src.typeTable.at(
      getProp(node, "type"));
  const auto name =
    llvm::cast<XcodeMl::ClassType>(resultT.get())->name();
  assert(name.hasValue());
  auto children = findNodes(node, "*[position() > 1]", src.ctxt);
    // ignore first child, which represents the result (class) type of
    // the clang::CXXTemporaryObjectExpr
  std::vector<CodeFragment> args;
  for (auto child : children) {
    args.push_back(w.walk(child, src));
  }
  return *name +
    makeTokenNode("(") +
    join(",", args) +
    makeTokenNode(")");
}

const ClangClassHandler ClangStmtHandler(
    "class",
    cxxgen::makeInnerNode,
    callCodeBuilder,
    {
      { "CallExpr", callExprProc },
      { "CXXMemberCallExpr", CXXMemberCallExprProc },
      { "CXXTemporaryObjectExpr", CXXTemporaryObjectExprProc },
    });

DEFINE_CCH(FriendDeclProc) {
  if (auto TL = findFirst(node, "TypeLoc", src.ctxt)) {
    /* friend class declaration */
    const auto dtident = getProp(TL, "type");
    const auto T = src.typeTable.at(dtident);
    return makeTokenNode("friend")
      + makeDecl(T, cxxgen::makeVoidNode(), src.typeTable)
      + makeTokenNode(";");
  }
  return
    makeTokenNode("friend") +
    callCodeBuilder(node, w, src);
}

const ClangClassHandler ClangDeclHandler(
    "class",
    cxxgen::makeInnerNode,
    callCodeBuilder,
    {
      { "Friend", FriendDeclProc },
    });
