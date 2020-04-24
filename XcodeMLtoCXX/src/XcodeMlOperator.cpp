#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <libxml/debugXML.h>
#include <libxml/tree.h>
#include "llvm/ADT/Optional.h"
#include "LibXMLUtil.h"
#include "StringTree.h"
#include "Util.h"
#include "XcodeMlNns.h"

#include "XcodeMlOperator.h"

const std::map<std::string, std::string> opMap = {
    {"plusExpr", "+"},
    {"minusExpr", "-"},
    {"mulExpr", "*"},
    {"divExpr", "/"},
    {"modExpr", "%"},
    {"LshiftExpr", "<<"},
    {"RshiftExpr", ">>"},
    {"bitAndExpr", "&"},
    {"bitOrExpr", "|"},
    {"bitXorExpr", "^"},
    {"commaExpr", ","},

    {"preIncrExpr", "++"},
    {"preDecrExpr", "--"},
    {"postIncrExpr", "++"},
    {"postDecrExpr", "--"},

    {"assignExpr", "="},
    {"asgPlusExpr", "+="},
    {"asgMinusExpr", "-="},
    {"asgMulExpr", "*="},
    {"asgDivExpr", "/="},
    {"asgModExpr", "%="},
    {"asgLshiftExpr", "<<="},
    {"asgRshiftExpr", ">>="},
    {"asgBitAndExpr", "&="},
    {"asgBitOrExpr", "|="},
    {"asgBitXorExpr", "^="},

    {"arrowExpr", "->"},
    {"arrowStarExpr", "->*"},
    {"callExpr", "()"},
    {"subScriptExpr", "[]"},

    {"logEQExpr", "=="},
    {"logNEQExpr", "!="},
    {"logGEExpr", ">="},
    {"logGTExpr", ">"},
    {"logLEExpr", "<="},
    {"logLTExpr", "<"},
    {"logAndExpr", "&&"},
    {"logOrExpr", "||"},
    {"newExpr", "new"},
    {"newArrayExpr", "new[]"},
    {"deleteExpr", "delete"},
    {"deleteArrayExpr", "delete[]"},
    {"AddrOfExpr", "&"},
    {"pointerRef", "*"},
    {"memberIndirectRef", "::*"},
    {"memberPointerRef", ".*"},
    {"unaryPlusExpr", "+"},
    {"unaryMinusExpr", "-"},
    {"logNotExpr", "!"},
    {"bitNotExpr", "~"},
};

namespace XcodeMl {

llvm::Optional<std::string>
OperatorNameToSpelling(const std::string &opName) {
  return getOrNull(opMap, opName);
}

XcodeMl::CodeFragment
makeOpNode(xmlNodePtr operatorNode) {
  const auto opName = getContent(operatorNode);
  const auto op = XcodeMl::OperatorNameToSpelling(opName);
  if (!op.hasValue()) {
    const auto lineno = xmlGetLineNo(operatorNode);
    assert(lineno >= 0);
    std::cerr << "Unknown operator name: '" << opName << "'" << std::endl
              << "lineno: " << lineno << std::endl;
    xmlDebugDumpNode(stderr, operatorNode, 0);
    std::abort();
  }
  return CXXCodeGen::makeTokenNode(*op);
}

} // namespace XcodeMl
