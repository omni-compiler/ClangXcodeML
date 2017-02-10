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
#include "Symbol.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "SourceInfo.h"
#include "CodeBuilder.h"
#include "ClangClassHandler.h"

#define CCH_ARGS xmlNodePtr node __attribute__((unused)), \
                 const CodeBuilder& w __attribute__((unused)), \
                 SourceInfo& src __attribute__((unused)), \
                 std::stringstream& ss __attribute__((unused))

#define DEFINE_CCH(name) static void name(CCH_ARGS)

DEFINE_CCH(callExprProc) {
  (w["functionCall"])(w, node, src, ss);
}

const ClangClassHandler ClangStmtHandler(
    "class",
    {
      { "CallExpr", callExprProc },
    });
