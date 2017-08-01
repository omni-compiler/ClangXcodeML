#include <iostream>
#include <map>
#include <string>
#include "llvm/ADT/Optional.h"
#include "Util.h"

#include "XcodeMlOperator.h"

const std::map<std::string, std::string>
opMap = {
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

  {"logEQExpr", "=="},
  {"logNEQExpr", "!="},
  {"logGEExpr", ">="},
  {"logGTExpr", ">"},
  {"logLEExpr", "<="},
  {"logLTExpr", "<"},
  {"logAndExpr", "&&"},
  {"logOrExpr", "||"},
};

namespace XcodeMl {

llvm::Optional<std::string>
OperatorNameToSpelling(const std::string& opName) {
  return getOrNull(opMap, opName);
}

} // namespace XcodeMl
