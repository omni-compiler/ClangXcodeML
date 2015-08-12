#include "XcodeMlVisitorBase.h"
#include "TypeTableVisitor.h"
#include "SymbolsVisitor.h"
#include "DeclarationsVisitor.h"

using namespace clang;
using namespace llvm;

static cl::opt<bool>
OptTraceDeclarations("trace-declarations",
                     cl::desc("emit traces on <globalDeclarations>, <declarations>"),
                     cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableDeclarations("disable-declarations",
                       cl::desc("disable <globalDeclarations>, <declarations>"),
                       cl::cat(C2XcodeMLCategory));

const char *
DeclarationsVisitor::getVisitorName() const {
  return OptTraceDeclarations ? "Declarations" : nullptr;
}

// helper macros

#define NType(mes) do {                          \
    newProp("type", "Type_" mes);                \
    newProp("pointer", typenamestr.c_str());     \
    return true;                                 \
  } while (0)

bool
DeclarationsVisitor::WrapExpr(Stmt *S) {
  Expr *E = dyn_cast<Expr>(S);
  if (E) {
    newChild("exprStatement");
    setLocation(E->getExprLoc());
  }
  if (TraverseMeStmt(S)) {
    if (!E
        && S->getStmtClass() != Stmt::CompoundStmtClass
        && S->getStmtClass() != Stmt::DeclStmtClass
        && S->getStmtClass() != Stmt::NullStmtClass
        && S->getStmtClass() != Stmt::LabelStmtClass
        && S->getStmtClass() != Stmt::CaseStmtClass
        && S->getStmtClass() != Stmt::DefaultStmtClass
        && S->getStmtClass() != Stmt::IndirectGotoStmtClass) {
      setLocation(S->getLocStart());
    }
    return true;
  } else {
    return false;
  }
}

void
DeclarationsVisitor::WrapChild(const char **names) {
  HookForStmt = [this, names](Stmt *S){
      DeclarationsVisitor V(this);

      if (names[1]) {
        WrapChild(names + 1);
      } else {
        HookForStmt = nullptr;
      }

      if (!S) {
        // ignore this nullptr
        newComment("an ignored nullptr exists");
        return true;
      } else if (names[0][0] == '-') {
        // ignore this child
        newComment("an ignored child exists");
        return true;
      } else if (names[0][0] == '+') {
        if (names[0][1] != '\0') {
          V.newChild(names[0] + 1);
        }
        return V.WrapExpr(S);
      } else if (names[0][0] == '*') {
        if (names[0][1] != '\0') {
          V.newChild(names[0] + 1);
        }
        if (S->getStmtClass() == Stmt::CompoundStmtClass) {
          V.WrapCompoundStatementBody(curNode, false);
          return V.TraverseChildOfStmt(S);
        } else {
          return V.WrapExpr(S);
        }
      } else {
        if (names[0][0] != '\0') {
          V.newChild(names[0]);
        }
        return V.TraverseMeStmt(S);
      }
  };
}

void
DeclarationsVisitor::PropChild(const char *name) {
  HookForDeclarationNameInfo = [this, name](DeclarationNameInfo NI){
      DeclarationsVisitor V(this);
      DeclarationName DN = NI.getName();
      IdentifierInfo *II = DN.getAsIdentifierInfo();
      newProp(name, II ? II->getNameStart() : "");
      HookForDeclarationNameInfo = nullptr;
      return true;
  };
}

void
DeclarationsVisitor::NameChild(const char *name, Expr *E) {
  HookForDeclarationNameInfo = [this, name, E](DeclarationNameInfo NI){
      DeclarationsVisitor V(this);
      DeclarationName DN = NI.getName();
      IdentifierInfo *II = DN.getAsIdentifierInfo();
      newChild(name, II ? II->getNameStart() : nullptr);
      TraverseType(E->getType());
      HookForDeclarationNameInfo = nullptr;
      return true;
  };
}

void
DeclarationsVisitor::WrapCompoundStatementBody(xmlNodePtr compoundStatement,
                                               bool nowInDeclPart) {
  HookForStmt = [this, compoundStatement, nowInDeclPart](Stmt *S){
      bool nowInBodyPart;

      if (S->getStmtClass() == Stmt::DeclStmtClass) {
        if (nowInDeclPart) {
          newComment("Stmt::DeclStmtClass: nowInDeclPart=true");
          WrapCompoundStatementBody(compoundStatement, true);
        } else {
          newComment("Stmt::DeclStmtClass: nowInDeclPart=false");
          newChild("compoundStatement");
          setLocation(S->getLocStart());
          SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
          SV.TraverseChildOfStmt(S);
          WrapCompoundStatementBody(curNode, true);
          newChild("declarations");
        }
        nowInBodyPart = false;
      } else {
        if (nowInDeclPart) {
          curNode = compoundStatement;
          newChild("body");
        }
        WrapCompoundStatementBody(compoundStatement, false);
        nowInBodyPart = true;
      }
      DeclarationsVisitor V(this);
      return V.WrapExpr(S);
  };
}

void
DeclarationsVisitor::WrapLabelChild(void) {
  HookForStmt = [this](Stmt *S){
      DeclarationsVisitor V(this);
      Expr *E = dyn_cast<Expr>(S);
      if (E) {
        V.newChild("exprStatement");
        V.setLocation(E->getExprLoc());
      }
      HookForStmt = nullptr;
      if (V.TraverseMeStmt(S)) {
        if (!E
            && S->getStmtClass() != Stmt::NullStmtClass
            && S->getStmtClass() != Stmt::LabelStmtClass) {
          V.setLocation(S->getLocStart());
        }
        return true;
      } else {
        return false;
      }
  };
}

#define NStmt(mes) do {newChild(mes); return true;} while (0)
#define NStmtXXX(mes) NStmt("Stmt_" mes)
#define NExpr(mes, content) do {                                        \
    newChild(mes, content);                                             \
    TraverseType(static_cast<Expr*>(S)->getType());                     \
    return true;                                                        \
  } while (0)
bool
DeclarationsVisitor::PreVisitStmt(Stmt *S) {
  if (!S) {
    newComment("Stmt_NULL");
    return false;
  }
  HookForAttr = [this](Attr *A){
      newChild("gccAttributes");
      HookForAttr = nullptr;
      return TraverseAttr(A);
  };

  const BinaryOperator *BO = dyn_cast<const BinaryOperator>(S);

  if (BO) {
    // XcodeML-C-0.9J.pdf: 7.6(assignExpr), 7.7, 7.10(commmaExpr)
    //QualType T = BO->getType();
    switch (BO->getOpcode()) {
    case BO_PtrMemD:   NExpr("UNDEF_BO_PtrMemD", nullptr);
    case BO_PtrMemI:   NExpr("UNDEF_BO_PtrMemI", nullptr);
    case BO_Mul:       NExpr("mulExpr", nullptr);
    case BO_Div:       NExpr("divExpr", nullptr);
    case BO_Rem:       NExpr("modExpr", nullptr);
    case BO_Add:       NExpr("plusExpr", nullptr);
    case BO_Sub:       NExpr("minusExpr", nullptr);
    case BO_Shl:       NExpr("LshiftExpr", nullptr);
    case BO_Shr:       NExpr("RshiftExpr", nullptr);
    case BO_LT:        NExpr("logLTExpr", nullptr);
    case BO_GT:        NExpr("logGTExpr", nullptr);
    case BO_LE:        NExpr("logLEExpr", nullptr);
    case BO_GE:        NExpr("logGEExpr", nullptr);
    case BO_EQ:        NExpr("logEQExpr", nullptr);
    case BO_NE:        NExpr("logNEQExpr", nullptr);
    case BO_And:       NExpr("bitAndExpr", nullptr);
    case BO_Xor:       NExpr("bitXorExpr", nullptr);
    case BO_Or:        NExpr("bitOrExpr", nullptr);
    case BO_LAnd:      NExpr("logAndExpr", nullptr);
    case BO_LOr:       NExpr("logOrExpr", nullptr);
    case BO_Assign:    NExpr("assignExpr", nullptr);
    case BO_Comma:     NExpr("commaExpr", nullptr);
    case BO_MulAssign: NExpr("asgMulExpr", nullptr);
    case BO_DivAssign: NExpr("asgDivExpr", nullptr);
    case BO_RemAssign: NExpr("asgModExpr", nullptr);
    case BO_AddAssign: NExpr("asgPlusExpr", nullptr);
    case BO_SubAssign: NExpr("asgMinusExpr", nullptr);
    case BO_ShlAssign: NExpr("asgLshiftExpr", nullptr);
    case BO_ShrAssign: NExpr("asgRshiftExpr", nullptr);
    case BO_AndAssign: NExpr("asgBitAndExpr", nullptr);
    case BO_OrAssign:  NExpr("asgBitOrExpr", nullptr);
    case BO_XorAssign: NExpr("asgBitXorExpr", nullptr);
    }
  }
  const UnaryOperator *UO = dyn_cast<const UnaryOperator>(S);
  if (UO) {
    // XcodeML-C-0.9J.pdf 7.2(varAddr), 7.3(pointerRef), 7.8, 7.11
    //QualType T = UO->getType();
    switch (UO->getOpcode()) {
    case UO_PostInc:   NExpr("postIncrExpr", nullptr);
    case UO_PostDec:   NExpr("postDecrExpr", nullptr);
    case UO_PreInc:    NExpr("preIncrExpr", nullptr);
    case UO_PreDec:    NExpr("preDecrExpr", nullptr);
    case UO_AddrOf:    NExpr("varAddr", nullptr);
    case UO_Deref:     NExpr("pointerRef", nullptr);
    case UO_Plus:      NExpr("UNDEF_UO_Plus", nullptr);
    case UO_Minus:     NExpr("unaryMinusExpr", nullptr);
    case UO_Not:       NExpr("bitNotExpr", nullptr);
    case UO_LNot:      NExpr("logNotExpr", nullptr);
    case UO_Real:      NExpr("UNDEF_UO_Real", nullptr);
    case UO_Imag:      NExpr("UNDEF_UO_Imag", nullptr);
    case UO_Extension: NExpr("UNDEF_UO_Extension", nullptr);
    }
  }

  switch (S->getStmtClass()) {
  case Stmt::NoStmtClass:     NStmtXXX("NoStmtClass");
  case Stmt::GCCAsmStmtClass: NStmtXXX("GCCAsmStmtClass");
  case Stmt::MSAsmStmtClass:  NStmtXXX("MSAsmStmtClass");
  case Stmt::AttributedStmtClass: NStmtXXX("AttributedStmtClass");
  case Stmt::BreakStmtClass: NStmt("breakStatement"); //6.7
  case Stmt::CXXCatchStmtClass: NStmtXXX("CXXCatchStmtClass");
  case Stmt::CXXForRangeStmtClass: NStmtXXX("CXXForRangeStmtClass");
  case Stmt::CXXTryStmtClass: NStmtXXX("CXXTryStmtClass");
  case Stmt::CapturedStmtClass: NStmtXXX("CapturedStmtClass");
  case Stmt::CompoundStmtClass: {
    // 6.2
    newChild("compoundStatement");
    setLocation(S->getLocStart());
    SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
    SV.TraverseChildOfStmt(S);
    WrapCompoundStatementBody(curNode, true);
    NStmt("declarations");
  }
  case Stmt::ContinueStmtClass:
    //6.8
    NStmt("continueStatement");
  case Stmt::DeclStmtClass:
    return true; // everything is performed by WrapCompoundStatementBody
  case Stmt::DoStmtClass: {
    //6.5
    static const char *childnames[] = {"+body", "condition", nullptr};
    WrapChild(childnames);
    NStmt("doStatement");
  }
  case Stmt::BinaryConditionalOperatorClass:
    //7.13
    NExpr("condExpr", nullptr);
  case Stmt::ConditionalOperatorClass:
    //7.13
    NExpr("condExpr", nullptr);
  case Stmt::AddrLabelExprClass: NStmtXXX("AddrLabelExprClass");
  case Stmt::ArraySubscriptExprClass:
    //7.4 (this cannot support C++)
    NStmtXXX("ArraySubscriptExprClass");
  case Stmt::ArrayTypeTraitExprClass: NStmtXXX("ArrayTypeTraitExprClass");
  case Stmt::AsTypeExprClass: NStmtXXX("AsTypeExprClass");
  case Stmt::AtomicExprClass: NStmtXXX("AtomicExprClass");
  case Stmt::BinaryOperatorClass: NStmtXXX("BinaryOperatorClass");
  case Stmt::CompoundAssignOperatorClass: NStmtXXX("CompoundAssignOperatorClass");
  case Stmt::BlockExprClass: NStmtXXX("BlockExprClass");
  case Stmt::CXXBindTemporaryExprClass: NStmtXXX("CXXBindTemporaryExprClass");
  case Stmt::CXXBoolLiteralExprClass: NStmtXXX("CXXBoolLiteralExprClass");
  case Stmt::CXXConstructExprClass: NStmtXXX("CXXConstructExprClass");
  case Stmt::CXXTemporaryObjectExprClass: NStmtXXX("CXXTemporaryObjectExprClass");
  case Stmt::CXXDefaultArgExprClass: NStmtXXX("CXXDefaultArgExprClass");
  case Stmt::CXXDefaultInitExprClass: NStmtXXX("CXXDefaultInitExprClass");
  case Stmt::CXXDeleteExprClass: NStmtXXX("CXXDeleteExprClass");
  case Stmt::CXXDependentScopeMemberExprClass: NStmtXXX("CXXDependentScopeMemberExprClass");
  case Stmt::CXXFoldExprClass: NStmtXXX("CXXFoldExprClass");
  case Stmt::CXXNewExprClass: NStmtXXX("CXXNewExprClass");
  case Stmt::CXXNoexceptExprClass: NStmtXXX("CXXNoexceptExprClass");
  case Stmt::CXXNullPtrLiteralExprClass: NStmtXXX("CXXNullPtrLiteralExprClass");
  case Stmt::CXXPseudoDestructorExprClass: NStmtXXX("CXXPseudoDestructorExprClass");
  case Stmt::CXXScalarValueInitExprClass: NStmtXXX("CXXScalarValueInitExprClass");
  case Stmt::CXXStdInitializerListExprClass: NStmtXXX("CXXStdInitializerListExprClass");
  case Stmt::CXXThisExprClass: NStmtXXX("CXXThisExprClass");
  case Stmt::CXXThrowExprClass: NStmtXXX("CXXThrowExprClass");
  case Stmt::CXXTypeidExprClass: NStmtXXX("CXXTypeidExprClass");
  case Stmt::CXXUnresolvedConstructExprClass: NStmtXXX("CXXUnresolvedConstructExprClass");
  case Stmt::CXXUuidofExprClass: NStmtXXX("CXXUuidofExprClass");
  case Stmt::CallExprClass:
    //7.9
    HookForStmt = [this](Stmt *S){
        optContext.nameForDeclRefExpr = "function";
        HookForStmt = [this](Stmt *S){
          optContext.nameForDeclRefExpr = nullptr;
          newChild("arguments");
          HookForStmt = nullptr;
          return TraverseStmt(S);
        };
        return TraverseStmt(S);
    };
    newChild("functionCall");
    TraverseType(static_cast<Expr*>(S)->getType());
    return true;
  case Stmt::CUDAKernelCallExprClass: NStmtXXX("CUDAKernelCallExprClass");
  case Stmt::CXXMemberCallExprClass: NStmtXXX("CXXMemberCallExprClass");
  case Stmt::CXXOperatorCallExprClass: NStmtXXX("CXXOperatorCallExprClass");
  case Stmt::UserDefinedLiteralClass: NStmtXXX("UserDefinedLiteralClass");
  case Stmt::CStyleCastExprClass: NStmt("castExpr"); //7.12
  case Stmt::CXXFunctionalCastExprClass: NStmtXXX("CXXFunctionalCastExprClass");
  case Stmt::CXXConstCastExprClass: NStmtXXX("CXXConstCastExprClass");
  case Stmt::CXXDynamicCastExprClass: NStmtXXX("CXXDynamicCastExprClass");
  case Stmt::CXXReinterpretCastExprClass: NStmtXXX("CXXReinterpretCastExprClass");
  case Stmt::CXXStaticCastExprClass: NStmtXXX("CXXStaticCastExprClass");
  case Stmt::ObjCBridgedCastExprClass: NStmtXXX("ObjCBridgedCastExprClass");
  case Stmt::ImplicitCastExprClass:
    //NStmtXXX("ImplicitCastExprClass");
    /// XXX : experimental
    newComment("Stmt_ImplicitCastExprClass");
    if (!optContext.nameForDeclRefExpr) {
      optContext.nameForDeclRefExpr = "Var";
    }
    return true;
  case Stmt::CharacterLiteralClass: NStmtXXX("CharacterLiteralClass");
  case Stmt::ChooseExprClass: NStmtXXX("ChooseExprClass");
  case Stmt::CompoundLiteralExprClass: NStmtXXX("CompoundLiteralExprClass");
  case Stmt::ConvertVectorExprClass: NStmtXXX("ConvertVectorExprClass");
  case Stmt::DeclRefExprClass:
    if (optContext.nameForDeclRefExpr) {
      NameChild(optContext.nameForDeclRefExpr, static_cast<Expr*>(S));
    } else {
      NameChild("varAddr", static_cast<Expr*>(S));
    }
    newComment("Stmt_DeclRefExprClass");
    return true;
  case Stmt::DependentScopeDeclRefExprClass: NStmtXXX("DependentScopeDeclRefExprClass");
  case Stmt::DesignatedInitExprClass: NStmtXXX("DesignatedInitExprClass");
  case Stmt::ExprWithCleanupsClass: NStmtXXX("ExprWithCleanupsClass");
  case Stmt::ExpressionTraitExprClass: NStmtXXX("ExpressionTraitExprClass");
  case Stmt::ExtVectorElementExprClass: NStmtXXX("ExtVectorElementExprClass");
  case Stmt::FloatingLiteralClass: {
    //7.1
    std::string valueAsString;
#if 0
    double Value = static_cast<FloatingLiteral*>(S)->getValueAsApproximateDouble();
    raw_string_ostream OS(valueAsString);

    OS << Value;
    OS.str();
#else
    valueAsString = contentBySource(S->getLocStart(), S->getLocEnd());
#endif
    NExpr("floatConstant", valueAsString.c_str());
  }
  case Stmt::FunctionParmPackExprClass: NStmtXXX("FunctionParmPackExprClass");
  case Stmt::GNUNullExprClass: NStmtXXX("GNUNullExprClass");
  case Stmt::GenericSelectionExprClass: NStmtXXX("GenericSelectionExprClass");
  case Stmt::ImaginaryLiteralClass: NStmtXXX("ImaginaryLiteralClass");
  case Stmt::ImplicitValueInitExprClass: NStmtXXX("ImplicitValueInitExprClass");
  case Stmt::InitListExprClass: NStmtXXX("InitListExprClass");
  case Stmt::IntegerLiteralClass: {
    //7.1 XXX: long long should be treated specially
    APInt Value = static_cast<IntegerLiteral*>(S)->getValue();
    std::string valueAsString;
    raw_string_ostream OS(valueAsString);
    OS << *Value.getRawData();
    OS.str();
    NExpr("intConstant", valueAsString.c_str());
  }
  case Stmt::LambdaExprClass: NStmtXXX("LambdaExprClass");
  case Stmt::MSPropertyRefExprClass: NStmtXXX("MSPropertyRefExprClass");
  case Stmt::MaterializeTemporaryExprClass: NStmtXXX("MaterializeTemporaryExprClass");
  case Stmt::MemberExprClass:
    //7.5 (TBD: how to handle C++ "->" overloadding)
    PropChild("member");
    optContext.nameForDeclRefExpr = nullptr;
    NExpr("memberRef", nullptr); 
  case Stmt::ObjCArrayLiteralClass: NStmtXXX("ObjCArrayLiteralClass");
  case Stmt::ObjCBoolLiteralExprClass: NStmtXXX("ObjCBoolLiteralExprClass");
  case Stmt::ObjCBoxedExprClass: NStmtXXX("ObjCBoxedExprClass");
  case Stmt::ObjCDictionaryLiteralClass: NStmtXXX("ObjCDictionaryLiteralClass");
  case Stmt::ObjCEncodeExprClass: NStmtXXX("ObjCEncodeExprClass");
  case Stmt::ObjCIndirectCopyRestoreExprClass: NStmtXXX("ObjCIndirectCopyRestoreExprClass");
  case Stmt::ObjCIsaExprClass: NStmtXXX("ObjCIsaExprClass");
  case Stmt::ObjCIvarRefExprClass: NStmtXXX("ObjCIvarRefExprClass");
  case Stmt::ObjCMessageExprClass: NStmtXXX("ObjCMessageExprClass");
  case Stmt::ObjCPropertyRefExprClass: NStmtXXX("ObjCPropertyRefExprClass");
  case Stmt::ObjCProtocolExprClass: NStmtXXX("ObjCProtocolExprClass");
  case Stmt::ObjCSelectorExprClass: NStmtXXX("ObjCSelectorExprClass");
  case Stmt::ObjCStringLiteralClass: NStmtXXX("ObjCStringLiteralClass");
  case Stmt::ObjCSubscriptRefExprClass: NStmtXXX("ObjCSubscriptRefExprClass");
  case Stmt::OffsetOfExprClass: NStmtXXX("OffsetOfExprClass");
  case Stmt::OpaqueValueExprClass: NStmtXXX("OpaqueValueExprClass");
  case Stmt::UnresolvedLookupExprClass: NStmtXXX("UnresolvedLookupExprClass");
  case Stmt::UnresolvedMemberExprClass: NStmtXXX("UnresolvedMemberExprClass");
  case Stmt::PackExpansionExprClass: NStmtXXX("PackExpansionExprClass");
  case Stmt::ParenExprClass: return true;; // no explicit node
  case Stmt::ParenListExprClass: NStmtXXX("ParenListExprClass");
  case Stmt::PredefinedExprClass: NStmtXXX("PredefinedExprClass");
  case Stmt::PseudoObjectExprClass: NStmtXXX("PseudoObjectExprClass");
  case Stmt::ShuffleVectorExprClass: NStmtXXX("ShuffleVectorExprClass");
  case Stmt::SizeOfPackExprClass: NStmtXXX("SizeOfPackExprClass");
  case Stmt::StmtExprClass:
    // 7.14
    NStmt("gccCompoundExpr");
  case Stmt::StringLiteralClass: {
    //7.1
    StringRef Data = static_cast<StringLiteral*>(S)->getString();
    std::string literalAsString;
    raw_string_ostream OS(literalAsString);

    for (unsigned i = 0, e = Data.size(); i != e; ++i) {
      unsigned char C = Data[i];
      if (C == '"' || C == '\\') {
        OS << '\\' << (char)C;
        continue;
      }
      if (isprint(C)) {
        OS << (char)C;
        continue;
      }
      switch (C) {
      case '\b': OS << "\\b"; break;
      case '\f': OS << "\\f"; break;
      case '\n': OS << "\\n"; break;
      case '\r': OS << "\\r"; break;
      case '\t': OS << "\\t"; break;
      default:
        OS << '\\';
        OS << ((C >> 6) & 0x7) + '0';
        OS << ((C >> 3) & 0x7) + '0';        
        OS << ((C >> 0) & 0x7) + '0';
        break;
      }
    }
    OS.str();
    NExpr("stringConstant", literalAsString.c_str());
  }
  case Stmt::SubstNonTypeTemplateParmExprClass: NStmtXXX("SubstNonTypeTemplateParmExprClass");
  case Stmt::SubstNonTypeTemplateParmPackExprClass: NStmtXXX("SubstNonTypeTemplateParmPackExprClass");
  case Stmt::TypeTraitExprClass: NStmtXXX("TypeTraitExprClass");
  case Stmt::TypoExprClass: NStmtXXX("TypoExprClass");
  case Stmt::UnaryExprOrTypeTraitExprClass: NStmtXXX("UnaryExprOrTypeTraitExprClass");
  case Stmt::UnaryOperatorClass: NStmtXXX("UnaryOperatorClass");
  case Stmt::VAArgExprClass: NStmtXXX("VAArgExprClass");
  case Stmt::ForStmtClass: {
    //6.6
    static const char *childnames[] = {
      "init",
      "-", // Variable Declaration
      "condition", "iter", "+body", nullptr
    };
    WrapChild(childnames);
    NStmt("forStatement");
  }
  case Stmt::GotoStmtClass: {
    //6.10
    LabelDecl *LD = static_cast<GotoStmt*>(S)->getLabel();
    newChild("gotoStatement");
    newChild("name", LD->getNameAsString().c_str());
    return false; // no need to traverse children recursively
  }
  case Stmt::IfStmtClass: {
    //6.3
    static const char *childnames[] = {
      "-", // Variable Declaration
      "condition", "+then", "+else", nullptr
    };
    WrapChild(childnames);
    NStmt("ifStatement");
  }
  case Stmt::IndirectGotoStmtClass:
    //6.10
    newChild("gotoStatement");
    setLocation(S->getLocStart());
    newChild("pointerRef"); // XXX?
    return true;
  case Stmt::LabelStmtClass: {
    //6.11
    LabelDecl *LD = static_cast<LabelStmt*>(S)->getDecl();
    xmlNodePtr origCurNode = curNode;
    newChild("statementLabel");
    setLocation(S->getLocStart());
    newChild("name", LD->getNameAsString().c_str());
    curNode = origCurNode;
    WrapLabelChild();
    return true; // create children (statement) to my parent directly
  }
  case Stmt::MSDependentExistsStmtClass: NStmtXXX("MSDependentExistsStmtClass");
  case Stmt::NullStmtClass:
    // not explicitly defined in specification,
    // but C_Front does not discard null statements silently.
    return false; // do not traverse children
  case Stmt::OMPAtomicDirectiveClass: NStmtXXX("OMPAtomicDirectiveClass");
  case Stmt::OMPBarrierDirectiveClass: NStmtXXX("OMPBarrierDirectiveClass");
  case Stmt::OMPCriticalDirectiveClass: NStmtXXX("OMPCriticalDirectiveClass");
  case Stmt::OMPFlushDirectiveClass: NStmtXXX("OMPFlushDirectiveClass");
  case Stmt::OMPForDirectiveClass: NStmtXXX("OMPForDirectiveClass");
  case Stmt::OMPForSimdDirectiveClass: NStmtXXX("OMPForSimdDirectiveClass");
  case Stmt::OMPParallelForDirectiveClass: NStmtXXX("OMPParallelForDirectiveClass");
  case Stmt::OMPParallelForSimdDirectiveClass: NStmtXXX("OMPParallelForSimdDirectiveClass");
  case Stmt::OMPSimdDirectiveClass: NStmtXXX("OMPSimdDirectiveClass");
  case Stmt::OMPMasterDirectiveClass: NStmtXXX("OMPMasterDirectiveClass");
  case Stmt::OMPOrderedDirectiveClass: NStmtXXX("OMPOrderedDirectiveClass");
  case Stmt::OMPParallelDirectiveClass: NStmtXXX("OMPParallelDirectiveClass");
  case Stmt::OMPParallelSectionsDirectiveClass: NStmtXXX("OMPParallelSectionsDirectiveClass");
  case Stmt::OMPSectionDirectiveClass: NStmtXXX("OMPSectionDirectiveClass");
  case Stmt::OMPSectionsDirectiveClass: NStmtXXX("OMPSectionsDirectiveClass");
  case Stmt::OMPSingleDirectiveClass: NStmtXXX("OMPSingleDirectiveClass");
  case Stmt::OMPTargetDirectiveClass: NStmtXXX("OMPTargetDirectiveClass");
  case Stmt::OMPTaskDirectiveClass: NStmtXXX("OMPTaskDirectiveClass");
  case Stmt::OMPTaskwaitDirectiveClass: NStmtXXX("OMPTaskwaitDirectiveClass");
  case Stmt::OMPTaskyieldDirectiveClass: NStmtXXX("OMPTaskyieldDirectiveClass");
  case Stmt::OMPTeamsDirectiveClass: NStmtXXX("OMPTeamsDirectiveClass");
  case Stmt::ObjCAtCatchStmtClass: NStmtXXX("ObjCAtCatchStmtClass");
  case Stmt::ObjCAtFinallyStmtClass: NStmtXXX("ObjCAtFinallyStmtClass");
  case Stmt::ObjCAtSynchronizedStmtClass: NStmtXXX("ObjCAtSynchronizedStmtClass");
  case Stmt::ObjCAtThrowStmtClass: NStmtXXX("ObjCAtThrowStmtClass");
  case Stmt::ObjCAtTryStmtClass: NStmtXXX("ObjCAtTryStmtClass");
  case Stmt::ObjCAutoreleasePoolStmtClass: NStmtXXX("ObjCAutoreleasePoolStmtClass");
  case Stmt::ObjCForCollectionStmtClass: NStmtXXX("ObjCForCollectionStmtClass");
  case Stmt::ReturnStmtClass:
    //6.9
    NStmt("returnStatement");
  case Stmt::SEHExceptStmtClass: NStmtXXX("SEHExceptStmtClass");
  case Stmt::SEHFinallyStmtClass: NStmtXXX("SEHFinallyStmtClass");
  case Stmt::SEHLeaveStmtClass: NStmtXXX("SEHLeaveStmtClass");
  case Stmt::SEHTryStmtClass: NStmtXXX("SEHTryStmtClass");
  case Stmt::CaseStmtClass: {
    //6.13, 6.14
    CaseStmt *CS = static_cast<CaseStmt *>(S);
    Expr *LHS = CS->getLHS();
    Expr *RHS = CS->getRHS();
    xmlNodePtr origCurNode = curNode;
    if (RHS) {
      newChild("gccRangedCaseLabel");
      setLocation(S->getLocStart());
      xmlNodePtr labelNode = curNode;
      newChild("value");
      TraverseStmt(LHS);
      curNode = labelNode;
      newChild("value");
      TraverseStmt(RHS);
    } else {
      newChild("caseLabel");
      setLocation(S->getLocStart());
      newChild("value");
      TraverseStmt(LHS);
    }
    // ignore the first two children
    static const char *childnames[] = {"-", "-", "+", nullptr};
    WrapChild(childnames);

    // the child stmt should belongs to my parent directly
    curNode = origCurNode;
    return true; 
  }
  case Stmt::DefaultStmtClass: {
    //6.15
    xmlNodePtr origCurNode = curNode;
    newChild("defaultLabel");
    setLocation(S->getLocStart());
    static const char *childnames[] = {"+", nullptr};
    WrapChild(childnames);

    // the child stmt should belongs to my parent directly
    curNode = origCurNode;
    return true;
  }
  case Stmt::SwitchStmtClass: {
    //6.12
    static const char *childnames[] = {
      "-", // Variable Declaration
      "value", "*body", nullptr
    };
    WrapChild(childnames);
    NStmt("switchStatement");
  }
  case Stmt::WhileStmtClass: {
    //6.4
    static const char *childnames[] = {
      "-", // Variable Declaration
      "condition", "+body", nullptr
    };
    WrapChild(childnames);
    NStmt("whileStatement");
  }
  }
}
#undef NStmt
#undef NStmtXXX
#undef NExpr

#define NType(mes) do {                          \
    newProp("type", "Type_" mes);                \
    newProp("pointer", typenamestr.c_str());     \
    return true;                                 \
  } while (0)
bool
DeclarationsVisitor::PreVisitType(QualType T) {
  if (T.isNull()) {
    newComment("Type_NULL");
    return false;
  }

  newProp("type", typetableinfo->getTypeName(T).c_str());
  if (T.isConstQualified()) {
    newProp("is_const", "1");
  }
  if (T.isVolatileQualified()) {
    newProp("is_volatile", "1");
  }
  return false; // do not traverse children

#if 0
  const Type *Tptr = T.getTypePtrOrNull();

  // XXX experimental code
  std::string typenamestr;
  raw_string_ostream OS(typenamestr);
  OS << (const void*)Tptr;
  OS.str();

  switch (T->getTypeClass()) {
  case Type::Builtin: {
    ASTContext &CXT = mangleContext->getASTContext();
    PrintingPolicy PP(CXT.getLangOpts());
    newProp("type",
            static_cast<const BuiltinType*>(Tptr)->getName(PP).str().c_str());
    if (T.isConstQualified()) {
      newProp("is_const", "1");
    }
    if (T.isVolatileQualified()) {
      newProp("is_volatile", "1");
    }
    return true;
  }
  case Type::Complex: NType("Complex");
  case Type::Pointer: NType("Pointer");
  case Type::BlockPointer: NType("BlockPointer");
  case Type::LValueReference: NType("LValueReference");
  case Type::RValueReference: NType("RValueReference");
  case Type::MemberPointer: NType("MemberPointer");
  case Type::ConstantArray: NType("ConstantArray");
  case Type::IncompleteArray: NType("IncompleteArray");
  case Type::VariableArray: NType("VariableArray");
  case Type::DependentSizedArray: NType("DependentSizedArray");
  case Type::DependentSizedExtVector: NType("DependentSizedExtVector");
  case Type::Vector: NType("Vector");
  case Type::ExtVector: NType("ExtVector");
  case Type::FunctionProto: NType("FunctionProto");
  case Type::FunctionNoProto: NType("FunctionNoProto");
  case Type::UnresolvedUsing: NType("UnresolvedUsing");
  case Type::Paren: NType("Paren");
  case Type::Typedef: NType("Typedef");
  case Type::Adjusted: NType("Adjusted");
  case Type::Decayed: NType("Decayed");
  case Type::TypeOfExpr: NType("TypeOfExpr");
  case Type::TypeOf: NType("TypeOf");
  case Type::Decltype: NType("Decltype");
  case Type::UnaryTransform: NType("UnaryTransform");
  case Type::Record: NType("Record");
  case Type::Enum: NType("Enum");
  case Type::Elaborated: NType("Elaborated");
  case Type::Attributed: NType("Attributed");
  case Type::TemplateTypeParm: NType("TemplateTypeParm");
  case Type::SubstTemplateTypeParm: NType("SubstTemplateTypeParm");
  case Type::SubstTemplateTypeParmPack: NType("SubstTemplateTypeParmPack");
  case Type::TemplateSpecialization: NType("TemplateSpecialization");
  case Type::Auto: NType("Auto");
  case Type::InjectedClassName: NType("InjectedClassName");
  case Type::DependentName: NType("DependentName");
  case Type::DependentTemplateSpecialization: NType("DependentTemplateSpecialization");
  case Type::PackExpansion: NType("PackExpansion");
  case Type::ObjCObject: NType("ObjCObject");
  case Type::ObjCInterface: NType("ObjCInterface");
  case Type::ObjCObjectPointer: NType("ObjCObjectPointer");
  case Type::Atomic: NType("Atomic");
  }
#endif
}
#undef NType

#if 0
#define NTypeLoc(mes) do {                                       \
    std::string content;                                         \
    newChild("TypeLoc_" mes);                                    \
    content = contentBySource(TL.getLocStart(), TL.getLocEnd()); \
    addChild("source", content.c_str());                         \
    return true;                                                 \
  } while (0)
#else
#define NTypeLoc(mes) do {                                       \
    newComment("TypeLoc_" mes);                                  \
    return true;                                                 \
  } while (0)
#endif
bool
DeclarationsVisitor::PreVisitTypeLoc(TypeLoc TL) {
  if (TL.isNull()) {
    newComment("TypeLoc_NULL");
    return false;
  }

  switch (TL.getTypeLocClass()) {
  case TypeLoc::Qualified: NTypeLoc("Qualified");
  case TypeLoc::Builtin: NTypeLoc("Builtin");
  case TypeLoc::Complex: NTypeLoc("Complex");
  case TypeLoc::Pointer: NTypeLoc("Pointer");
  case TypeLoc::BlockPointer: NTypeLoc("BlockPointer");
  case TypeLoc::LValueReference: NTypeLoc("LValueReference");
  case TypeLoc::RValueReference: NTypeLoc("RValueReference");
  case TypeLoc::MemberPointer: NTypeLoc("MemberPointer");
  case TypeLoc::ConstantArray: NTypeLoc("ConstantArray");
  case TypeLoc::IncompleteArray: NTypeLoc("IncompleteArray");
  case TypeLoc::VariableArray: NTypeLoc("VariableArray");
  case TypeLoc::DependentSizedArray: NTypeLoc("DependentSizedArray");
  case TypeLoc::DependentSizedExtVector: NTypeLoc("DependentSizedExtVector");
  case TypeLoc::Vector: NTypeLoc("Vector");
  case TypeLoc::ExtVector: NTypeLoc("ExtVector");
  case TypeLoc::FunctionProto: NTypeLoc("FunctionProto");
  case TypeLoc::FunctionNoProto: NTypeLoc("FunctionNoProto");
  case TypeLoc::UnresolvedUsing: NTypeLoc("UnresolvedUsing");
  case TypeLoc::Paren: NTypeLoc("Paren");
  case TypeLoc::Typedef: NTypeLoc("Typedef");
  case TypeLoc::Adjusted: NTypeLoc("Adjusted");
  case TypeLoc::Decayed: NTypeLoc("Decayed");
  case TypeLoc::TypeOfExpr: NTypeLoc("TypeOfExpr");
  case TypeLoc::TypeOf: NTypeLoc("TypeOf");
  case TypeLoc::Decltype: NTypeLoc("Decltype");
  case TypeLoc::UnaryTransform: NTypeLoc("UnaryTransform");
  case TypeLoc::Record: NTypeLoc("Record");
  case TypeLoc::Enum: NTypeLoc("Enum");
  case TypeLoc::Elaborated: NTypeLoc("Elaborated");
  case TypeLoc::Attributed: NTypeLoc("Attributed");
  case TypeLoc::TemplateTypeParm: NTypeLoc("TemplateTypeParm");
  case TypeLoc::SubstTemplateTypeParm: NTypeLoc("SubstTemplateTypeParm");
  case TypeLoc::SubstTemplateTypeParmPack: NTypeLoc("SubstTemplateTypeParmPack");
  case TypeLoc::TemplateSpecialization: NTypeLoc("TemplateSpecialization");
  case TypeLoc::Auto: NTypeLoc("Auto");
  case TypeLoc::InjectedClassName: NTypeLoc("InjectedClassName");
  case TypeLoc::DependentName: NTypeLoc("DependentName");
  case TypeLoc::DependentTemplateSpecialization: NTypeLoc("DependentTemplateSpecialization");
  case TypeLoc::PackExpansion: NTypeLoc("PackExpansion");
  case TypeLoc::ObjCObject: NTypeLoc("ObjCObject");
  case TypeLoc::ObjCInterface: NTypeLoc("ObjCInterface");
  case TypeLoc::ObjCObjectPointer: NTypeLoc("ObjCObjectPointer");
  case TypeLoc::Atomic: NTypeLoc("Atomic");
  }
}
#undef NTypeLoc

#define NAttr(mes) newComment("Attr_" mes); break
bool
DeclarationsVisitor::PreVisitAttr(Attr *A) {
  if (!A) {
    newComment("Attr_NULL");
    return false;
  }
  switch (A->getKind()) {
  case attr::NUM_ATTRS: NAttr("NUMATTRS"); // may not be used
  case attr::AMDGPUNumSGPR: NAttr("AMDGPUNumSGPR");
  case attr::AMDGPUNumVGPR: NAttr("AMDGPUNumVGPR");
  case attr::ARMInterrupt: NAttr("ARMInterrupt");
  case attr::AcquireCapability: NAttr("AcquireCapability");
  case attr::AcquiredAfter: NAttr("AcquiredAfter");
  case attr::AcquiredBefore: NAttr("AcquiredBefore");
  case attr::Alias: NAttr("Alias");
  case attr::AlignMac68k: NAttr("AlignMac68k");
  case attr::AlignValue: NAttr("AlignValue");
  case attr::Aligned: NAttr("Aligned");
  case attr::AlwaysInline: NAttr("AlwaysInline");
  case attr::AnalyzerNoReturn: NAttr("AnalyzerNoReturn");
  case attr::Annotate: NAttr("Annotate");
  case attr::ArcWeakrefUnavailable: NAttr("ArcWeakrefUnavailable");
  case attr::ArgumentWithTypeTag: NAttr("ArgumentWithTypeTag");
  case attr::AsmLabel: NAttr("AsmLabel");
  case attr::AssertCapability: NAttr("AssertCapability");
  case attr::AssertExclusiveLock: NAttr("AssertExclusiveLock");
  case attr::AssertSharedLock: NAttr("AssertSharedLock");
  case attr::AssumeAligned: NAttr("AssumeAligned");
  case attr::Availability: NAttr("Availability");
  case attr::Blocks: NAttr("Blocks");
  case attr::C11NoReturn: NAttr("C11NoReturn");
  case attr::CDecl: NAttr("CDecl");
  case attr::CFAuditedTransfer: NAttr("CFAuditedTransfer");
  case attr::CFConsumed: NAttr("CFConsumed");
  case attr::CFReturnsNotRetained: NAttr("CFReturnsNotRetained");
  case attr::CFReturnsRetained: NAttr("CFReturnsRetained");
  case attr::CFUnknownTransfer: NAttr("CFUnknownTransfer");
  case attr::CUDAConstant: NAttr("CUDAConstant");
  case attr::CUDADevice: NAttr("CUDADevice");
  case attr::CUDAGlobal: NAttr("CUDAGlobal");
  case attr::CUDAHost: NAttr("CUDAHost");
  case attr::CUDAInvalidTarget: NAttr("CUDAInvalidTarget");
  case attr::CUDALaunchBounds: NAttr("CUDALaunchBounds");
  case attr::CUDAShared: NAttr("CUDAShared");
  case attr::CXX11NoReturn: NAttr("CXX11NoReturn");
  case attr::CallableWhen: NAttr("CallableWhen");
  case attr::Capability: NAttr("Capability");
  case attr::CapturedRecord: NAttr("CapturedRecord");
  case attr::CarriesDependency: NAttr("CarriesDependency");
  case attr::Cleanup: NAttr("Cleanup");
  case attr::Cold: NAttr("Cold");
  case attr::Common: NAttr("Common");
  case attr::Const: NAttr("Const");
  case attr::Constructor: NAttr("Constructor");
  case attr::Consumable: NAttr("Consumable");
  case attr::ConsumableAutoCast: NAttr("ConsumableAutoCast");
  case attr::ConsumableSetOnRead: NAttr("ConsumableSetOnRead");
  case attr::DLLExport: NAttr("DLLExport");
  case attr::DLLImport: NAttr("DLLImport");
  case attr::Deprecated: NAttr("Deprecated");
  case attr::Destructor: NAttr("Destructor");
  case attr::EnableIf: NAttr("EnableIf");
  case attr::ExclusiveTrylockFunction: NAttr("ExclusiveTrylockFunction");
  case attr::FallThrough: NAttr("FallThrough");
  case attr::FastCall: NAttr("FastCall");
  case attr::Final: NAttr("Final");
  case attr::Flatten: NAttr("Flatten");
  case attr::Format: NAttr("Format");
  case attr::FormatArg: NAttr("FormatArg");
  case attr::GNUInline: NAttr("GNUInline");
  case attr::GuardedBy: NAttr("GuardedBy");
  case attr::GuardedVar: NAttr("GuardedVar");
  case attr::Hot: NAttr("Hot");
  case attr::IBAction: NAttr("IBAction");
  case attr::IBOutlet: NAttr("IBOutlet");
  case attr::IBOutletCollection: NAttr("IBOutletCollection");
  case attr::InitPriority: NAttr("InitPriority");
  case attr::InitSeg: NAttr("InitSeg");
  case attr::IntelOclBicc: NAttr("IntelOclBicc");
  case attr::LockReturned: NAttr("LockReturned");
  case attr::LocksExcluded: NAttr("LocksExcluded");
  case attr::LoopHint: NAttr("LoopHint");
  case attr::MSABI: NAttr("MSABI");
  case attr::MSInheritance: NAttr("MSInheritance");
  case attr::MSP430Interrupt: NAttr("MSP430Interrupt");
  case attr::MSVtorDisp: NAttr("MSVtorDisp");
  case attr::Malloc: NAttr("Malloc");
  case attr::MaxFieldAlignment: NAttr("MaxFieldAlignment");
  case attr::MayAlias: NAttr("MayAlias");
  case attr::MinSize: NAttr("MinSize");
  case attr::Mips16: NAttr("Mips16");
  case attr::Mode: NAttr("Mode");
  case attr::MsStruct: NAttr("MsStruct");
  case attr::NSConsumed: NAttr("NSConsumed");
  case attr::NSConsumesSelf: NAttr("NSConsumesSelf");
  case attr::NSReturnsAutoreleased: NAttr("NSReturnsAutoreleased");
  case attr::NSReturnsNotRetained: NAttr("NSReturnsNotRetained");
  case attr::NSReturnsRetained: NAttr("NSReturnsRetained");
  case attr::Naked: NAttr("Naked");
  case attr::NoCommon: NAttr("NoCommon");
  case attr::NoDebug: NAttr("NoDebug");
  case attr::NoDuplicate: NAttr("NoDuplicate");
  case attr::NoInline: NAttr("NoInline");
  case attr::NoInstrumentFunction: NAttr("NoInstrumentFunction");
  case attr::NoMips16: NAttr("NoMips16");
  case attr::NoReturn: NAttr("NoReturn");
  case attr::NoSanitizeAddress: NAttr("NoSanitizeAddress");
  case attr::NoSanitizeMemory: NAttr("NoSanitizeMemory");
  case attr::NoSanitizeThread: NAttr("NoSanitizeThread");
  case attr::NoSplitStack: NAttr("NoSplitStack");
  case attr::NoThreadSafetyAnalysis: NAttr("NoThreadSafetyAnalysis");
  case attr::NoThrow: NAttr("NoThrow");
  case attr::NonNull: NAttr("NonNull");
  case attr::OMPThreadPrivateDecl: NAttr("OMPThreadPrivateDecl");
  case attr::ObjCBridge: NAttr("ObjCBridge");
  case attr::ObjCBridgeMutable: NAttr("ObjCBridgeMutable");
  case attr::ObjCBridgeRelated: NAttr("ObjCBridgeRelated");
  case attr::ObjCDesignatedInitializer: NAttr("ObjCDesignatedInitializer");
  case attr::ObjCException: NAttr("ObjCException");
  case attr::ObjCExplicitProtocolImpl: NAttr("ObjCExplicitProtocolImpl");
  case attr::ObjCMethodFamily: NAttr("ObjCMethodFamily");
  case attr::ObjCNSObject: NAttr("ObjCNSObject");
  case attr::ObjCPreciseLifetime: NAttr("ObjCPreciseLifetime");
  case attr::ObjCRequiresPropertyDefs: NAttr("ObjCRequiresPropertyDefs");
  case attr::ObjCRequiresSuper: NAttr("ObjCRequiresSuper");
  case attr::ObjCReturnsInnerPointer: NAttr("ObjCReturnsInnerPointer");
  case attr::ObjCRootClass: NAttr("ObjCRootClass");
  case attr::ObjCRuntimeName: NAttr("ObjCRuntimeName");
  case attr::OpenCLImageAccess: NAttr("OpenCLImageAccess");
  case attr::OpenCLKernel: NAttr("OpenCLKernel");
  case attr::OptimizeNone: NAttr("OptimizeNone");
  case attr::Overloadable: NAttr("Overloadable");
  case attr::Override: NAttr("Override");
  case attr::Ownership: NAttr("Ownership");
  case attr::Packed: NAttr("Packed");
  case attr::ParamTypestate: NAttr("ParamTypestate");
  case attr::Pascal: NAttr("Pascal");
  case attr::Pcs: NAttr("Pcs");
  case attr::PnaclCall: NAttr("PnaclCall");
  case attr::PtGuardedBy: NAttr("PtGuardedBy");
  case attr::PtGuardedVar: NAttr("PtGuardedVar");
  case attr::Pure: NAttr("Pure");
  case attr::ReleaseCapability: NAttr("ReleaseCapability");
  case attr::ReqdWorkGroupSize: NAttr("ReqdWorkGroupSize");
  case attr::RequiresCapability: NAttr("RequiresCapability");
  case attr::ReturnTypestate: NAttr("ReturnTypestate");
  case attr::ReturnsNonNull: NAttr("ReturnsNonNull");
  case attr::ReturnsTwice: NAttr("ReturnsTwice");
  case attr::ScopedLockable: NAttr("ScopedLockable");
  case attr::Section: NAttr("Section");
  case attr::SelectAny: NAttr("SelectAny");
  case attr::Sentinel: NAttr("Sentinel");
  case attr::SetTypestate: NAttr("SetTypestate");
  case attr::SharedTrylockFunction: NAttr("SharedTrylockFunction");
  case attr::StdCall: NAttr("StdCall");
  case attr::SysVABI: NAttr("SysVABI");
  case attr::TLSModel: NAttr("TLSModel");
  case attr::TestTypestate: NAttr("TestTypestate");
  case attr::ThisCall: NAttr("ThisCall");
  case attr::Thread: NAttr("Thread");
  case attr::TransparentUnion: NAttr("TransparentUnion");
  case attr::TryAcquireCapability: NAttr("TryAcquireCapability");
  case attr::TypeTagForDatatype: NAttr("TypeTagForDatatype");
  case attr::TypeVisibility: NAttr("TypeVisibility");
  case attr::Unavailable: NAttr("Unavailable");
  case attr::Unused: NAttr("Unused");
  case attr::Used: NAttr("Used");
  case attr::Uuid: NAttr("Uuid");
  case attr::VecReturn: NAttr("VecReturn");
  case attr::VecTypeHint: NAttr("VecTypeHint");
  case attr::VectorCall: NAttr("VectorCall");
  case attr::Visibility: NAttr("Visibility");
  case attr::WarnUnused: NAttr("WarnUnused");
  case attr::WarnUnusedResult: NAttr("WarnUnusedResult");
  case attr::Weak: NAttr("Weak");
  case attr::WeakImport: NAttr("WeakImport");
  case attr::WeakRef: NAttr("WeakRef");
  case attr::WorkGroupSizeHint: NAttr("WorkGroupSizeHint");
  case attr::X86ForceAlignArgPointer: NAttr("X86ForceAlignArgPointer");
  }
  newChild("gccAttribute");

  newProp("name", contentBySource(A->getLocation(), A->getLocation()).c_str());

  std::string prettyprint;
  raw_string_ostream OS(prettyprint);
  ASTContext &CXT = mangleContext->getASTContext();
  A->printPretty(OS, PrintingPolicy(CXT.getLangOpts()));
  newComment(OS.str().c_str());

  return true;
}
#undef NAttr

#define NDeclXXX(mes) do {                      \
    newChild("Decl_" mes);                      \
    setLocation(D->getLocation());              \
    return true;                                \
  } while (0)
bool
DeclarationsVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return false;
  }
  switch (D->getKind()) {
  case Decl::AccessSpec: NDeclXXX("AccessSpec");
  case Decl::Block: NDeclXXX("Block");
  case Decl::Captured: NDeclXXX("Captured");
  case Decl::ClassScopeFunctionSpecialization: NDeclXXX("ClassScopeFunctionSpecialization");
  case Decl::Empty: NDeclXXX("Empty");
  case Decl::FileScopeAsm: NDeclXXX("FileScopeAsm");
  case Decl::Friend: NDeclXXX("Friend");
  case Decl::FriendTemplate: NDeclXXX("FriendTemplate");
  case Decl::Import: NDeclXXX("Import");
  case Decl::LinkageSpec: NDeclXXX("LinkageSpec");
  case Decl::Label: NDeclXXX("Label");
  case Decl::Namespace: NDeclXXX("Namespace");
  case Decl::NamespaceAlias: NDeclXXX("NamespaceAlias");
  case Decl::ObjCCompatibleAlias: NDeclXXX("ObjCCompatibleAlias");
  case Decl::ObjCCategory: NDeclXXX("ObjCCategory");
  case Decl::ObjCCategoryImpl: NDeclXXX("ObjCCategoryImpl");
  case Decl::ObjCImplementation: NDeclXXX("ObjCImplementation");
  case Decl::ObjCInterface: NDeclXXX("ObjCInterface");
  case Decl::ObjCProtocol: NDeclXXX("ObjCProtocol");
  case Decl::ObjCMethod: NDeclXXX("ObjCMethod");
  case Decl::ObjCProperty: NDeclXXX("ObjCProperty");
  case Decl::ClassTemplate: NDeclXXX("ClassTemplate");
  case Decl::FunctionTemplate: NDeclXXX("FunctionTemplate");
  case Decl::TypeAliasTemplate: NDeclXXX("TypeAliasTemplate");
  case Decl::VarTemplate: NDeclXXX("VarTemplate");
  case Decl::TemplateTemplateParm: NDeclXXX("TemplateTemplateParm");
  case Decl::Enum:
    newComment("Decl_Enum");
    return false; // to be traversed in typeTable
  case Decl::Record:
    newComment("Decl_Record");
    return false; // to be traversed in typeTable
  case Decl::CXXRecord:
    newComment("Decl_CXXRecord");
    return false; // to be traversed in typeTable
  case Decl::ClassTemplateSpecialization: NDeclXXX("ClassTemplateSpecialization");
  case Decl::ClassTemplatePartialSpecialization: NDeclXXX("ClassTemplatePartialSpecialization");
  case Decl::TemplateTypeParm: NDeclXXX("TemplateTypeParm");
  case Decl::TypeAlias: NDeclXXX("TypeAlias");
  case Decl::Typedef:
    //
    newComment("Decl_Typedef");
    return false; // to be traversed in typeTable
  case Decl::UnresolvedUsingTypename: NDeclXXX("UnresolvedUsingTypename");
  case Decl::Using: NDeclXXX("Using");
  case Decl::UsingDirective: NDeclXXX("UsingDirective");
  case Decl::UsingShadow: NDeclXXX("UsingShadow");
  case Decl::Field: NDeclXXX("Field");  // XXX?
  case Decl::ObjCAtDefsField: NDeclXXX("ObjCAtDefsField");
  case Decl::ObjCIvar: NDeclXXX("ObjCIvar");
  case Decl::Function: {
    // 5.1, 5.4 XXX
    FunctionDecl *FD = static_cast<FunctionDecl*>(D);
    if (FD && FD->isThisDeclarationADefinition()) {
      newChild("functionDefinition");
      setLocation(FD->getSourceRange().getBegin());
      xmlNodePtr functionNode = curNode;
      HookForDeclarationNameInfo = [this, D](DeclarationNameInfo NI) {
        DeclarationsVisitor V(this);
        if (V.TraverseDeclarationNameInfo(NI)) {
          SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
          SV.TraverseChildOfDecl(D);
          newChild("params"); //create a new node to parent just after NameInfo
          return true;
        } else {
          return false;
        }
      };
      bool IsValiadic = static_cast<FunctionDecl*>(D)->isVariadic();
      HookForStmt = [this, functionNode, IsValiadic](Stmt *S) {
        if (IsValiadic) {
          addChild("ellipsis");
        }
        curNode = functionNode;
        newChild("body");
        return TraverseStmt(S);
      };
    } else {
      newChild("functionDecl");
      HookForDeclarationNameInfo = [this, D](DeclarationNameInfo NI) {
        DeclarationsVisitor V(this);
        V.TraverseDeclarationNameInfo(NI);
        return false;
      };
    }
     
    return true;
  }
  case Decl::CXXMethod: NDeclXXX("CXXMethod");
  case Decl::CXXConstructor: NDeclXXX("CXXConstructor");
  case Decl::CXXConversion: NDeclXXX("CXXConversion");
  case Decl::CXXDestructor: NDeclXXX("CXXDestructor");
  case Decl::MSProperty: NDeclXXX("MSProperty");
  case Decl::NonTypeTemplateParm: NDeclXXX("NonTypeTemplateParm");
  case Decl::Var:
    // 5.3
    newChild("varDecl");
    return true;
  case Decl::ImplicitParam: NDeclXXX("ImplicitParam");
  case Decl::ParmVar: {
    ParmVarDecl *PVD = static_cast<ParmVarDecl*>(D);
    // 5.2 XXX
    newChild("name", PVD->getNameAsString().c_str());
    return true;
  }
  case Decl::VarTemplateSpecialization: NDeclXXX("VarTemplateSpecialization");
  case Decl::VarTemplatePartialSpecialization: NDeclXXX("VarTemplatePartialSpecialization");
  case Decl::EnumConstant: NDeclXXX("EnumConstant"); // XXX?
  case Decl::IndirectField: NDeclXXX("IndirectField");
  case Decl::UnresolvedUsingValue: NDeclXXX("UnresolvedUsingValue");
  case Decl::OMPThreadPrivate: NDeclXXX("OMPThreadPrivate");
  case Decl::ObjCPropertyImpl: NDeclXXX("ObjCPropertyImpl");
  case Decl::StaticAssert: NDeclXXX("StaticAssert");
  case Decl::TranslationUnit:
    if (OptDisableDeclarations) {
      return false; // stop traverse
    } else {
      return true; // no need to create a child
    }
  }
}
#undef NDecl
#undef NDeclXXX

#if 0
bool
DeclarationsVisitor::PreVisitDecl(Decl *D) {
  switch (D->getKind()) {
  case Decl::Function: {
    const NamedDecl *ND = dyn_cast<const NamedDecl>(D);
    if (ND) {
      raw_string_ostream OS(optContext.tmpstr);
      mangleContext->mangleName(ND, OS);
      xmlNewTextChild(curNode, nullptr,
                      BAD_CAST ")name", BAD_CAST OS.str().c_str());
    }
    break;
  }
  default:
    break;
  }
  return true;
}
#endif

#define NNNSXXX(mes) do {                       \
    newChild("NestedNameSpecifier_" mes);       \
    return true;                                \
  } while (0)
bool
DeclarationsVisitor::PreVisitNestedNameSpecifier(NestedNameSpecifier *NNS) {
  if (!NNS) {
    newComment("NestedNameSpecifier_NULL");
    return false;
  }
  switch (NNS->getKind()) {
  case NestedNameSpecifier::Identifier: NNNSXXX("Identifier");
  case NestedNameSpecifier::Namespace: NNNSXXX("Namespace");
  case NestedNameSpecifier::NamespaceAlias: NNNSXXX("NamespaceAlias");
  case NestedNameSpecifier::Global: NNNSXXX("Global");
  case NestedNameSpecifier::Super: NNNSXXX("Super");
  case NestedNameSpecifier::TypeSpec: NNNSXXX("TypeSpec");
  case NestedNameSpecifier::TypeSpecWithTemplate: NNNSXXX("TypeSpecWithTemplate");
  }
}
#undef NNNSXXX

#define NNNSLocXXX(mes) do {                    \
    newChild("NestedNameSpecifier_" mes);       \
    return true;                                \
  } while (0)
bool
DeclarationsVisitor::PreVisitNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS) {
  if (!NNS) {
    newComment("NestedNameSpecifierLoc_NULL");
    return false;
  }
  switch (NNS.getNestedNameSpecifier()->getKind()) {
  case NestedNameSpecifier::Identifier: NNNSLocXXX("Identifier");
  case NestedNameSpecifier::Namespace: NNNSLocXXX("Namespace");
  case NestedNameSpecifier::NamespaceAlias: NNNSLocXXX("NamespaceAlias");
  case NestedNameSpecifier::Global: NNNSLocXXX("Global");
  case NestedNameSpecifier::Super: NNNSLocXXX("Super");
  case NestedNameSpecifier::TypeSpec: NNNSLocXXX("TypeSpec");
  case NestedNameSpecifier::TypeSpecWithTemplate: NNNSLocXXX("TypeSpecWithTemplate");
  }
}
#undef NNNSLocXXX

#define NDeclName(mes) do {                                     \
    newComment("DeclarationNameInfo_" mes);                     \
    newChild("name", content);                                  \
    return true;                                                \
  } while (0)
bool
DeclarationsVisitor::PreVisitDeclarationNameInfo(DeclarationNameInfo NI) {
  DeclarationName DN = NI.getName();
  IdentifierInfo *II = DN.getAsIdentifierInfo();
  const char *content = II ? II->getNameStart() : nullptr;

  switch (DN.getNameKind()) {
  case DeclarationName::CXXConstructorName: NDeclName("CXXConstructorName");
  case DeclarationName::CXXDestructorName: NDeclName("CXXDestructorName");
  case DeclarationName::CXXConversionFunctionName: NDeclName("CXXConversionFunctionName");
  case DeclarationName::Identifier: NDeclName("Identifier");
  case DeclarationName::ObjCZeroArgSelector: NDeclName("ObjCZeroArgSelector");
  case DeclarationName::ObjCOneArgSelector: NDeclName("ObjCOneArgSelector");
  case DeclarationName::ObjCMultiArgSelector: NDeclName("ObjCMultiArgSelector");
  case DeclarationName::CXXOperatorName: NDeclName("CXXOperatorName");
  case DeclarationName::CXXLiteralOperatorName: NDeclName("CXXLiteralOperatorName");
  case DeclarationName::CXXUsingDirective: NDeclName("CXXUsingDirective");
  }
}
#undef NDeclName

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
