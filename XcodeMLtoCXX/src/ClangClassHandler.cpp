#include <algorithm>
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
#include "XcodeMlNns.h"
#include "XcodeMlName.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SourceInfo.h"
#include "CodeBuilder.h"
#include "ClangClassHandler.h"
#include "XcodeMlUtil.h"

namespace cxxgen = CXXCodeGen;

#define CCH_ARGS                                                              \
  xmlNodePtr node __attribute__((unused)),                                    \
      const CodeBuilder &w __attribute__((unused)),                           \
      SourceInfo &src __attribute__((unused))

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

DEFINE_CCH(CXXCtorExprProc) {
  return makeTokenNode("(") + cxxgen::join(", ", w.walkChildren(node, src))
      + makeTokenNode(")");
}

XcodeMl::CodeFragment
makeBases(XcodeMl::ClassType *T, SourceInfo &src) {
  using namespace XcodeMl;
  assert(T);
  const auto bases = T->getBases();
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
emitClassDefinition(xmlNodePtr node, const CodeBuilder &w, SourceInfo &src) {
  if (isTrueProp(node, "is_implicit", false)) {
    return cxxgen::makeVoidNode();
  }
  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto className = getQualifiedNameFromNameNode(nameNode, src);

  const auto typeName = getProp(node, "type");
  const auto type = src.typeTable.at(typeName);
  auto classType = llvm::dyn_cast<XcodeMl::ClassType>(type.get());
  assert(classType);
  classType->setName(className.toString(src.typeTable, src.nnsTable));

  std::vector<XcodeMl::CodeFragment> decls;

  for (xmlNodePtr memberNode = xmlFirstElementChild(node); memberNode;
       memberNode = xmlNextElementSibling(memberNode)) {
    const auto accessProp = getPropOrNull(memberNode, "access");
    if (!accessProp.hasValue()) {
      const auto decl =
          makeTokenNode("/* ignored a member with no access specifier */")
          + cxxgen::makeNewLineNode();
      decls.push_back(decl);
      continue;
    }
    const auto access = *accessProp;
    const auto decl = makeTokenNode(access) + makeTokenNode(":")
        + callCodeBuilder(memberNode, w, src);
    decls.push_back(decl);
  }

  return makeTokenNode("class")
      + className.toString(src.typeTable, src.nnsTable)
      + makeBases(classType, src) + makeTokenNode("{")
      + separateByBlankLines(decls) + makeTokenNode("}") + makeTokenNode(";")
      + cxxgen::makeNewLineNode();
}

DEFINE_CCH(CXXRecordProc) {
  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    return emitClassDefinition(node, w, src);
  }
  const auto T = src.typeTable.at(getProp(node, "type"));
  auto classT = llvm::dyn_cast<XcodeMl::ClassType>(T.get());
  assert(classT);
  const auto name = classT->name();
  assert(name.hasValue());
  return makeTokenNode("class") + (*name) + makeTokenNode(";");
}

DEFINE_CCH(CXXTemporaryObjectExprProc) {
  const auto resultT = src.typeTable.at(getProp(node, "type"));
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

const ClangClassHandler ClangStmtHandler("class",
    cxxgen::makeInnerNode,
    callCodeBuilder,
    {
        {"CallExpr", callExprProc},
        {"CXXConstructExpr", CXXCtorExprProc},
        {"CXXTemporaryObjectExpr", CXXTemporaryObjectExprProc},
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
  return makeTokenNode("friend") + callCodeBuilder(node, w, src);
}

const ClangClassHandler ClangDeclHandler("class",
    cxxgen::makeInnerNode,
    callCodeBuilder,
    {
        {"CXXRecord", CXXRecordProc}, {"Friend", FriendDeclProc},
    });
