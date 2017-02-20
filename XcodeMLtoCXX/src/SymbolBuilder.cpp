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
#include "CXXCodeGen.h"
#include "Symbol.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SourceInfo.h"
#include "TypeAnalyzer.h"
#include "SymbolBuilder.h"

namespace cxxgen = CXXCodeGen;

using SymbolBuilder = AttrProc<SourceInfo&, std::stringstream&>;

#define SB_ARGS xmlNodePtr node __attribute__((unused)), \
                SourceInfo& src __attribute__((unused)), \
                std::stringstream& ss __attribute__((unused))

#define DEFINE_SB(name) static void name(SB_ARGS)

DEFINE_SB(typedefNameProc) {
  const auto alias = getNameFromIdNode(node, src.ctxt);
  const auto type = src.typeTable.at(
      static_cast<XMLString>(xmlGetProp(node, BAD_CAST "type")));
  ss << "typedef "
     << makeDecl(type, alias, src.typeTable)
     << ";" << std::endl;
}

static void emitStructDefinition(
    const SourceInfo& src,
    const XcodeMl::TypeRef type,
    std::stringstream& ss
) {
  XcodeMl::Struct* structType = llvm::cast<XcodeMl::Struct>(type.get());
  ss << "struct " << structType->tagName() << "{" << std::endl;
  for (auto member : structType->members()) {
    const auto memberType = src.typeTable.at(member.type());
    ss << makeDecl(memberType, member.name(), src.typeTable);
    if (member.isBitField()) {
      ss << " : " << member.getSize();
    }
    ss << ";" << std::endl;
  }
  ss << "};" << std::endl;
}

DEFINE_SB(tagnameProc) {
  const auto tagname = getNameFromIdNode(node, src.ctxt);
  const auto type = src.typeTable.at(static_cast<XMLString>( xmlGetProp(node, BAD_CAST "type") ));
  emitStructDefinition(src, type, ss);
}

const SymbolBuilder CXXSymbolBuilder(
    "sclass",
    {
      { "typedef_name", typedefNameProc },
      { "tagname", tagnameProc },
    });

void buildSymbols(
    xmlNodePtr node,
    SourceInfo& src,
    std::stringstream& ss
) {
  CXXSymbolBuilder.walkAll(node, src, ss);
}
