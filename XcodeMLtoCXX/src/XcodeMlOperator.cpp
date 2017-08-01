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

std::string
OperatorNameToSpelling(const std::string& opName) {
  const auto spelling = getOrNull(opMap, opName);
  if (!spelling.hasValue()) {
    std::cerr << "Unknown operator name: '" << opName << "'"
      << std::endl;
    std::abort();
  }
  return *spelling;
}

} // namespace XcodeMl
