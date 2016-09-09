#include <functional>
#include <sstream>
#include <memory>
#include <map>
#include <cassert>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "LibXMLUtil.h"
#include "XMLString.h"
#include "XMLWalker.h"
#include "AttrProc.h"
#include "SymbolAnalyzer.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SourceInfo.h"
#include "TypeAnalyzer.h"
#include "SymbolsBuilder.h"

using SymbolsBuilder = AttrProc<SourceInfo&, std::stringstream&>;

#define SB_ARGS xmlNodePtr node __attribute__((unused)), \
                SourceInfo& src __attribute__((unused)), \
                std::stringstream& ss __attribute__((unused))

#define DEFINE_SB(name) void name(SB_ARGS)

static XMLString getName(xmlNodePtr idNode, xmlXPathContextPtr ctxt) {
  xmlNodePtr nameNode = findFirst(idNode, "name", ctxt);
  return xmlNodeGetContent(nameNode);
}

DEFINE_SB(typedefNameProc) {
  const auto alias = getName(node, src.ctxt);
  const auto type = src.typeTable.at(
      static_cast<XMLString>(xmlGetProp(node, BAD_CAST "type")));
  ss << "typedef "
     << makeDecl(type, alias, src.typeTable)
     << ";" << std::endl;
}

const SymbolsBuilder CXXSymbolsBuilder(
    "sclass",
    {
      { "typedef_name", typedefNameProc },
    });

void buildSymbols(
    xmlNodePtr node,
    SourceInfo& src,
    std::stringstream& ss
) {
  CXXSymbolsBuilder.walkAll(node, src, ss);
}
