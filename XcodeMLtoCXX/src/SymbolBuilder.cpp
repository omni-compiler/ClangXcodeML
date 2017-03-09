#include <functional>
#include <sstream>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "AttrProc.h"
#include "Stream.h"
#include "StringTree.h"
#include "Symbol.h"
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

using SymbolBuilder = AttrProc<StringTreeRef, SourceInfo&>;

#define SB_ARGS xmlNodePtr node __attribute__((unused)), \
                SourceInfo& src __attribute__((unused))

#define DEFINE_SB(name) static StringTreeRef name(SB_ARGS)

DEFINE_SB(typedefNameProc) {
  const auto alias = getNameFromIdNode(node, src.ctxt);
  const auto type = src.typeTable.at(
      static_cast<XMLString>(xmlGetProp(node, BAD_CAST "type")));
  return
    makeTokenNode("typedef") +
    makeDecl(type, makeTokenNode(alias), src.typeTable) +
    makeTokenNode(";");
}

static StringTreeRef emitStructDefinition(
    const SourceInfo& src,
    const XcodeMl::TypeRef type
) {
  XcodeMl::Struct* structType = llvm::cast<XcodeMl::Struct>(type.get());
  auto acc =
    makeTokenNode("struct") +
    structType->tagName() +
    makeTokenNode("{");
  for (auto member : structType->members()) {
    const auto memberType = src.typeTable.at(member.type());
    acc = acc + makeDecl(memberType, member.name(), src.typeTable);
    if (member.isBitField()) {
      acc = acc +
            makeTokenNode(":") +
            makeTokenNode(std::to_string(member.getSize()));
    }
    acc = acc + makeTokenNode(";");
  }
  return acc + makeTokenNode("};");
}

DEFINE_SB(tagnameProc) {
  const auto tagname = getNameFromIdNode(node, src.ctxt);
  const auto type = src.typeTable.at(static_cast<XMLString>( xmlGetProp(node, BAD_CAST "type") ));
  return emitStructDefinition(src, type);
}

const SymbolBuilder CXXSymbolBuilder(
    "sclass",
    makeInnerNode,
    makeVoidNode,
    {
      { "typedef_name", typedefNameProc },
      { "tagname", tagnameProc },
    });

StringTreeRef buildSymbols(
    xmlNodePtr node,
    SourceInfo& src
) {
  return separateByBlankLines(
      CXXSymbolBuilder.walkAll(node, src));
}
