#include <map>
#include <string>

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
  return opMap.at(opName);
}

} // namespace XcodeMl
