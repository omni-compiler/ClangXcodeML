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

using NnsAnalyzer = XMLWalker<
  void,
  xmlXPathContextPtr,
  XcodeMl::NnsMap&>;

#define NA_ARGS const NnsAnalyzer& w __attribute__((unused)), \
                xmlNodePtr node __attribute__((unused)), \
                xmlXPathContextPtr ctxt __attribute__((unused)), \
                XcodeMl::NnsMap & map __attribute__((unused))

#define DEFINE_NA(name) static void name(NA_ARGS)

DEFINE_NA(classNnsProc) {
  const auto name = getProp(node, "nns");
  const auto type = getProp(node, "type");
  map[name] = XcodeMl::makeClassNns(
      name,
      type);
}

const XcodeMl::NnsMap initialNnsMap = {
  { "global", XcodeMl::makeGlobalNns() },
};

const NnsAnalyzer XcodeMLNNSAnalyzer(
"NnsAnalyzer",
{
  { "classNNS", classNnsProc },
});

XcodeMl::NnsMap
analyzeNnsTable(
    xmlNodePtr nnsTable,
    xmlXPathContextPtr ctxt)
{
  XcodeMl::NnsMap map;
  if (nnsTable == nullptr) {
    return map;
  }

  auto nnsNodes = findNodes(nnsTable, "*", ctxt);
  for (auto nnsNode : nnsNodes) {
    XcodeMLNNSAnalyzer.walk(nnsNode, ctxt, map);
  }
  return map;
}
