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
#include "StringTree.h"
#include "Symbol.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SourceInfo.h"
#include "SymbolAnalyzer.h"

using SymbolAnalyzer =
    AttrProc<void, xmlXPathContextPtr, XcodeMl::Environment &>;

using CXXCodeGen::makeTokenNode;

#define SA_ARGS                                                               \
  xmlNodePtr node __attribute__((unused)),                                    \
      xmlXPathContextPtr ctxt __attribute__((unused)),                        \
      XcodeMl::Environment &map __attribute__((unused))

#define DEFINE_SA(name) static void name(SA_ARGS)

DEFINE_SA(tagnameProc) {
  const auto dataTypeIdent = getProp(node, "type");
  auto typeref = map.at(dataTypeIdent);

  const auto tagName = getNameFromIdNode(node, ctxt);

  if (auto ST = llvm::dyn_cast<XcodeMl::Struct>(typeref.get())) {
    ST->setTagName(makeTokenNode(tagName));
  } else if (auto ET = llvm::dyn_cast<XcodeMl::EnumType>(typeref.get())) {
    ET->setName(tagName);
  }
}

DEFINE_SA(classNameProc) {
  const auto dataTypeIdent = getProp(node, "type");
  auto typeref = map.at(dataTypeIdent);
  const auto name = getNameFromIdNode(node, ctxt);
  auto CT = llvm::dyn_cast<XcodeMl::ClassType>(typeref.get());
  assert(CT);
  CT->setName(name);
}

const SymbolAnalyzer CXXSymbolAnalyzer("sclass",
    {
        {"tagname", tagnameProc}, {"class_name", classNameProc},
    });

void
analyzeSymbols(
    xmlNodePtr node, xmlXPathContextPtr ctxt, XcodeMl::Environment &map) {
  CXXSymbolAnalyzer.walkAll(node, ctxt, map);
}
