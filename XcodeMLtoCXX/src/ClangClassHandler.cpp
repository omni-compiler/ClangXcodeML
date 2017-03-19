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

DEFINE_CCH(callCodeBuilder) {
  return makeInnerNode(w.walkChildren(node, src));
}

DEFINE_CCH(callExprProc) {
  return (w["functionCall"])(w, node, src);
}

DEFINE_CCH(CXXTemporaryObjectExprProc) {
  const auto resultT = src.typeTable.at(
      getProp(node, "type"));
  const auto name =
    llvm::cast<XcodeMl::ClassType>(resultT.get())->name();
  assert(name.hasValue());
  const auto args = w.walkChildren(node, src);
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
      { "CXXTemporaryObjectExpr", CXXTemporaryObjectExprProc },
    });
