#include "operator.h"
#include <map>

std::string OverloadedOperatorKindToString(clang::OverloadedOperatorKind op, unsigned param_size) {
  using namespace clang;
  const static std::map<OverloadedOperatorKind, std::string> unique_meaning = {
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
      return param_size == 2 ? "mulExpr" : "pointerRef"/*XXX: correct name?*/;
    case OO_Amp:
      return param_size == 2 ? "logAndExpr" : "varAddr"/*XXX: correct name?*/;
    case OO_PlusPlus:
      return param_size == 2 ? "postIncrExpr" : "preIncrExpr";
    case OO_MinusMinus:
      return param_size == 2 ? "postDecrExpr" : "preIncrExpr";
    default:
      return unique_meaning.at(op);
  }
}
