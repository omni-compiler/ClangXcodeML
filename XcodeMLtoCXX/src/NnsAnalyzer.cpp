#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "StringTree.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "XMLString.h"
#include "XMLWalker.h"

using NnsAnalyzer = XMLWalker<void, xmlXPathContextPtr, XcodeMl::NnsMap &>;

#define NA_ARGS                                                               \
  const NnsAnalyzer &w __attribute__((unused)),                               \
      xmlNodePtr node __attribute__((unused)),                                \
      xmlXPathContextPtr ctxt __attribute__((unused)),                        \
      XcodeMl::NnsMap &map __attribute__((unused))

#define DEFINE_NA(name) static void name(NA_ARGS)

DEFINE_NA(classNnsProc) {
  const auto name = getProp(node, "nns");
  const auto type = getProp(node, "type");
  const auto parent = getPropOrNull(node, "parent");
  if (parent.hasValue()) {
    map[name] = XcodeMl::makeClassNns(name, *parent, type);
  } else {
    // local classes
    map[name] = XcodeMl::makeClassNns(name, type);
  }
}

DEFINE_NA(otherNnsProc) {
  const auto nident = getProp(node, "nns");
  map[nident] = XcodeMl::makeOtherNns(nident);
}

DEFINE_NA(namespaceNnsProc) {
  const auto nident = getProp(node, "nns");
  const auto namespaceName = getContent(node);
  const auto parent = getProp(node, "parent");
  map[nident] = XcodeMl::makeNamespaceNns(nident, parent, namespaceName);
}

const XcodeMl::NnsMap initialNnsMap = {
    {"global", XcodeMl::makeGlobalNns()},
};

const NnsAnalyzer XcodeMLNNSAnalyzer("NnsAnalyzer",
    {
        std::make_tuple("classNNS", classNnsProc),
        std::make_tuple("classTemplateSpecializationNNS", classNnsProc),
        std::make_tuple("namespaceNNS", namespaceNnsProc),
        std::make_tuple("otherNNS", otherNnsProc),
    });

XcodeMl::NnsMap
analyzeNnsTable(xmlNodePtr nnsTable, xmlXPathContextPtr ctxt) {
  if (nnsTable == nullptr) {
    return initialNnsMap;
  }

  XcodeMl::NnsMap map = initialNnsMap;
  auto nnsNodes = findNodes(nnsTable, "*", ctxt);
  for (auto &&nnsNode : nnsNodes) {
    XcodeMLNNSAnalyzer.walk(nnsNode, ctxt, map);
  }
  return map;
}

XcodeMl::NnsMap
expandNnsMap(const XcodeMl::NnsMap &table,
    xmlNodePtr nnsTableNode,
    xmlXPathContextPtr ctxt) {
  auto newTable = table;
  const auto definitions = findNodes(nnsTableNode, "*", ctxt);
  for (auto &&definition : definitions) {
    XcodeMLNNSAnalyzer.walk(definition, ctxt, newTable);
  }
  return newTable;
}
