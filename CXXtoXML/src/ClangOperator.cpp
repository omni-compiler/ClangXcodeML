#include <map>
#include <string>
#include "clang/AST/OperationKinds.h"
#include "clang/Basic/OperatorKinds.h"

#include "ClangOperator.h"

const char*
BOtoElemName(clang::BinaryOperatorKind BO) {
  using namespace clang;
  const std::map<BinaryOperatorKind, const char*>
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

const char*
UOtoElemName(clang::UnaryOperatorKind UO) {
  using namespace clang;
  const std::map<UnaryOperatorKind, const char*>
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

const char*
OverloadedOperatorKindToString(
    clang::OverloadedOperatorKind op,
    unsigned param_size)
{
  using namespace clang;
  const static std::map<OverloadedOperatorKind, const char*>
    unique_meaning = {
    // 7.10 binary operators
    // arithmetic binary operators
    {OO_Slash,                "divExpr"},
    {OO_Percent,              "modExpr"},
    {OO_LessLess,             "LshiftExpr"},
    {OO_GreaterGreater,       "RshiftExpr"},
    {OO_Pipe,                 "bitOrExpr"},
    {OO_Caret,                "bitXorExpr"},

    // assignment operators
    {OO_PlusEqual,            "asgPlusExpr"},
    {OO_MinusEqual,           "asgMinusExpr"},
    {OO_StarEqual,            "asgMulExpr"},
    {OO_SlashEqual,           "asgDivExpr"},
    {OO_PercentEqual,         "asgModExpr"},
    {OO_LessLessEqual,        "asgLshiftExpr"},
    {OO_GreaterGreaterEqual,  "asgRshiftExpr"},
    {OO_AmpEqual,             "asgBitAndExpr"},
    {OO_PipeEqual,            "asgBitOrExpr"},
    {OO_CaretEqual,           "asgBitXorExpr"},

    // logical binary operators
    {OO_EqualEqual,           "logEQExpr"},
    {OO_ExclaimEqual,         "logNEQExpr"},
    {OO_GreaterEqual,         "logGEExpr"},
    {OO_Greater,              "logGTExpr"},
    {OO_LessEqual,            "logLEExpr"},
    {OO_Less,                 "logLTExpr"},
    {OO_AmpAmp,               "logAndExpr"},
      // rvalue reference operator(&&) is neither an operator nor overloadable
    {OO_PipePipe,             "logOrExpr"},
    {OO_Equal,                "assignExpr"},

    // 7.11 unary operators
    {OO_Tilde,                "bitNotExpr"},
    {OO_Exclaim,              "logNotExpr"},

    // 7.13 commaExpr element
    {OO_Comma,                "commaExpr"},

    // 7.19 newExpr and newArrayExpr elements
    // 7.20 deleteExpr and deleteArrayExpr elements
    {OO_New,                  "newExpr"},
    {OO_Array_New,            "newArrayExpr"},
    {OO_Delete,               "deleteExpr"},
    {OO_Array_Delete,         "deleteArrayExpr"},

    // XXX: undocumented yet, should be discussed
    {OO_Arrow,                "arrowExpr"},
    {OO_ArrowStar,            "arrowStarExpr"},
    {OO_Call,                 "callExpr"},
    {OO_Subscript,            "subScriptExpr"}
  };

  switch (op) {
    case OO_Plus:
      return param_size == 2 ? "plusExpr" : "unaryPlusExpr";
      // XXX: unray plus operator is not defined in document yet.
      // See 7.11 unary operators.
    case OO_Minus:
      return param_size == 2 ? "minusExpr" : "unaryMinusExpr";
    case OO_Star:
      return param_size == 2 ? "mulExpr" : "pointerRef"
        /*XXX: correct name?*/;
    case OO_Amp:
      return param_size == 2 ? "logAndExpr" : "varAddr"
        /*XXX: correct name?*/;
    case OO_PlusPlus:
      return param_size == 2 ? "postIncrExpr" : "preIncrExpr";
    case OO_MinusMinus:
      return param_size == 2 ? "postDecrExpr" : "preIncrExpr";
    default:
      return unique_meaning.at(op);
  }
}
