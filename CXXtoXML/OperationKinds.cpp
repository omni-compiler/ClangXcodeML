#include <map>
#include <string>
#include <experimental/optional>
#include "clang/AST/OperationKinds.h"

std::experimental::optional<std::string>
BOtoElemName(clang::BinaryOperatorKind BO) {
  using namespace clang;
  const std::map<BinaryOperatorKind, std::string>
    binOps = {
      {BO_PtrMemD, "memberPointerRef"},
      {BO_PtrMemI, "memberIndirectRef"}, // undefined by XcodeML
      {BO_Mul, "mulExpr"},
      {BO_Div, "divExpr"},
      {BO_Rem, "modExpr"},
      {BO_Add, "plusExpr"},
      {BO_Sub, "minusExpr"},
      {BO_Shl, "LshiftExpr"},
      {BO_Shr, "RshiftExpr"},
      {BO_LT, "logLTExpr"},
      {BO_GT, "logGTExpr"},
      {BO_LE, "logLEExpr"},
      {BO_GE, "logGEExpr"},
      {BO_EQ, "logEQExpr"},
      {BO_NE, "logNEQExpr"},
      {BO_And, "bitAndExpr"},
      {BO_Xor, "bitXorExpr"},
      {BO_Or, "bitOrExpr"},
      {BO_LAnd, "logAndExpr"},
      {BO_LOr, "logOrExpr"},
      {BO_Assign, "assignExpr"},
      {BO_Comma, "commaExpr"},
      {BO_MulAssign, "asgMulExpr"},
      {BO_DivAssign, "asgDivExpr"},
      {BO_RemAssign, "asgModExpr"},
      {BO_AddAssign, "asgPlusExpr"},
      {BO_SubAssign, "asgMinusExpr"},
      {BO_ShlAssign, "asgLshiftExpr"},
      {BO_ShrAssign, "asgRshiftExpr"},
      {BO_AndAssign, "asgBitAndExpr"},
      {BO_OrAssign, "asgBitOrExpr"},
      {BO_XorAssign, "asgBitXorExpr"},
  };
  auto iter = binOps.find(BO);
  return (iter == binOps.end() ? nullptr : iter->second);
}

std::experimental::optional<std::string>
UOtoElemName(clang::UnaryOperatorKind UO) {
  using namespace clang;
  const std::map<UnaryOperatorKind, std::string>
    unaryOps = {
      {UO_PostInc, "postIncrExpr"},
      {UO_PostDec, "postDecrExpr"},
      {UO_PreInc, "preIncrExpr"},
      {UO_PreDec, "preDecrExpr"},
      {UO_AddrOf, "AddrOfExpr"}, // undefined by XcodeML
      {UO_Deref, "pointerRef"},
      {UO_Plus, "unaryPlusExpr"},
      {UO_Minus, "unaryMinusExpr"},
      {UO_Not, "bitNotExpr"},
      {UO_LNot, "logNotExpr"},
      {UO_Real, "unaryRealExpr"}, // undefined by XcodeML
      {UO_Imag, "unaryImagExpr"}, // undefined by XcodeML
      {UO_Extension, "unrayExtensionExpr"}, // undefined by XcodeML
  };
  auto iter = unaryOps.find(UO);
  return (iter == unaryOps.end() ? nullptr : iter->second);
}
