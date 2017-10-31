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
#include "XMLString.h"
#include "XMLWalker.h"
#include "AttrProc.h"
#include "Stream.h"
#include "StringTree.h"
#include "Symbol.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SourceInfo.h"
#include "TypeAnalyzer.h"
#include "SymbolBuilder.h"

namespace cxxgen = CXXCodeGen;

using CXXCodeGen::StringTreeRef;
using CXXCodeGen::makeInnerNode;
using CXXCodeGen::makeTokenNode;
using CXXCodeGen::makeVoidNode;
using CXXCodeGen::separateByBlankLines;

using SymbolBuilder = AttrProc<StringTreeRef, SourceInfo &>;

#define SB_ARGS                                                               \
  xmlNodePtr node __attribute__((unused)),                                    \
      SourceInfo &src __attribute__((unused))

#define DEFINE_SB(name) static StringTreeRef name(SB_ARGS)

DEFINE_SB(NullProc) {
  return makeVoidNode();
}

DEFINE_SB(typedefNameProc) {
  if (isTrueProp(node, "is_implicit", false)) {
    return makeVoidNode();
  }
  const auto alias = getNameFromIdNode(node, src.ctxt);
  const auto type = src.typeTable.at(getProp(node, "type"));
  return makeTokenNode("typedef")
      + makeDecl(type, makeTokenNode(alias), src.typeTable)
      + makeTokenNode(";");
}

static StringTreeRef
emitStructDefinition(const SourceInfo &src, const XcodeMl::TypeRef type) {
  XcodeMl::Struct *structType = llvm::cast<XcodeMl::Struct>(type.get());
  return structType->makeStructDefinition(src.typeTable);
}

DEFINE_SB(tagnameProc) {
  const auto tagname = getNameFromIdNode(node, src.ctxt);
  const auto type = src.typeTable.at(getProp(node, "type"));
  return emitStructDefinition(src, type);
}

const SymbolBuilder CXXSymbolBuilder("sclass",
    makeInnerNode,
    NullProc,
    {
        std::make_tuple("typedef_name", typedefNameProc),
        std::make_tuple("tagname", tagnameProc),
    });

StringTreeRef
buildSymbols(xmlNodePtr node, SourceInfo &src) {
  return separateByBlankLines(CXXSymbolBuilder.walkAll(node, src));
}
