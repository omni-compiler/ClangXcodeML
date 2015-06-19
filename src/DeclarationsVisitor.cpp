#include "XcodeMlVisitorBase.h"
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

#define N(mes) do {newChild(mes); return true;} while (0)
#define NE(mes) do {                                                    \
    newChild(mes);                                                      \
    TraverseType(static_cast<Expr*>(S)->getType());                     \
    return true;                                                        \
  } while (0)

#define NT(mes) do {newChild(mes); TraverseType(T); return true;} while (0)
#define NS(mes) do {                                                    \
    newChild(mes);                                                      \
    return true;                                                        \
  } while (0)

#define ND(mes) do {newChild(mes); setLocation(D->getLocation()); return true;} while (0)

#define NTypeLoc(mes) do {                                      \
    newChild(mes);                                              \
    setContentBySource(TL.getLocStart(), TL.getLocEnd());       \
    addChild("source");                                         \
    return true;                                                \
  } while (0)

#define NAttr(mes) newComment("Attr_" mes); break

#define NType(mes) do {                          \
    newProp("type", "Type_" mes);                \
    newProp("pointer", contentString.c_str());   \
    return true;                                 \
  } while (0)

#define NDeclName(mes) do {                                     \
    newComment(mes);                                            \
    newChild("name");                                           \
    return true;                                                \
  } while (0)

void
DeclarationsVisitor::WrapChild(const char *name) {
  if (name[0] == '+') {
    HooksForStmt.push_back([this, name](Stmt *S){
        DeclarationsVisitor V(this);
        V.newChild(name + 1);
        return V.TraverseMeStmt(S);});
  } else {
    HooksForStmt.push_back([this, name](Stmt *S){
        DeclarationsVisitor V(this);
        V.newChild(name);
        return V.TraverseMeStmt(S);});
  }
}

void
DeclarationsVisitor::WrapChild(const char *name1, const char *name2,
                               const char *name3, const char *name4) {
  if (name4) {
    WrapChild(name4);
  }
  if (name3) {
    WrapChild(name3);
  }
  WrapChild(name2);
  WrapChild(name1);
}

void
DeclarationsVisitor::PropChild(const char *name) {
  HooksForDeclarationNameInfo.push_back([this, name](DeclarationNameInfo NI){
      DeclarationsVisitor V(this);
      DeclarationName DN = NI.getName();
      IdentifierInfo *II = DN.getAsIdentifierInfo();
      newProp(name, II ? II->getNameStart() : "");
      return true;});
}

void
DeclarationsVisitor::NameChild(const char *name) {
  HooksForDeclarationNameInfo.push_back([this, name](DeclarationNameInfo NI){
      DeclarationsVisitor V(this);
      V.optContext.explicitname = name;
      return V.TraverseMeDeclarationNameInfo(NI);});
}

void
DeclarationsVisitor::WrapCompoundStatementBody(xmlNodePtr compoundStatement,
                                               bool nowInDeclPart) {
  HooksForStmt.push_back([this, compoundStatement, nowInDeclPart](Stmt *S){
      bool nowInBodyPart;

      if (S->getStmtClass() == Stmt::DeclStmtClass) {
        if (nowInDeclPart) {
          newComment("Stmt::DeclStmtClass: nowInDeclPart=true");
          WrapCompoundStatementBody(compoundStatement, true);
        } else {
          newComment("Stmt::DeclStmtClass: nowInDeclPart=false");
          newChild("compoundStatement");
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
      Expr *E = dyn_cast<Expr>(S);
      if (E) {
        if (nowInBodyPart) {
          V.newChild("exprStatement");
          V.setLocation(E->getExprLoc());
        }
      }
      if (V.TraverseMeStmt(S)) {
        if (!E && S->getStmtClass() != Stmt::DeclStmtClass) {
          V.setLocation(S->getLocStart());
        }
        return true;
      } else {
        return false;
      }
    });
}

bool
DeclarationsVisitor::PreVisitStmt(Stmt *S) {
  if (!S) {
    newComment("Stmt_NULL");
    return false;
  }
  HooksForAttr.push_back([this](Attr *A){
      contentString = "";
      newChild("gccAttributes");
      return TraverseAttr(A);
    });

  const BinaryOperator *BO = dyn_cast<const BinaryOperator>(S);

  if (BO) {
    // XcodeML-C-0.9J.pdf: 7.6(assignExpr), 7.7, 7.10(commmaExpr)
    QualType T = BO->getType();
    switch (BO->getOpcode()) {
    case BO_PtrMemD:   NT("UNDEF_BO_PtrMemD");
    case BO_PtrMemI:   NT("UNDEF_BO_PtrMemI");
    case BO_Mul:       NT("mulExpr");
    case BO_Div:       NT("divExpr");
    case BO_Rem:       NT("modExpr");
    case BO_Add:       NT("plusExpr");
    case BO_Sub:       NT("minusExpr");
    case BO_Shl:       NT("LshiftExpr");
    case BO_Shr:       NT("RshiftExpr");
    case BO_LT:        NT("logLTExpr");
    case BO_GT:        NT("logGTExpr");
    case BO_LE:        NT("logLEExpr");
    case BO_GE:        NT("logGEExpr");
    case BO_EQ:        NT("logEQExpr");
    case BO_NE:        NT("logNEQExpr");
    case BO_And:       NT("bitAndExpr");
    case BO_Xor:       NT("bitXorExpr");
    case BO_Or:        NT("bitOrExpr");
    case BO_LAnd:      NT("logAndExpr");
    case BO_LOr:       NT("logOrExpr");
    case BO_Assign:    NT("assignExpr");
    case BO_Comma:     NT("commaExpr");
    case BO_MulAssign: NT("asgMulExpr");
    case BO_DivAssign: NT("asgDivExpr");
    case BO_RemAssign: NT("asgModExpr");
    case BO_AddAssign: NT("asgPlusExpr");
    case BO_SubAssign: NT("asgMinusExpr");
    case BO_ShlAssign: NT("asgLshiftExpr");
    case BO_ShrAssign: NT("asgRshiftExpr");
    case BO_AndAssign: NT("asgBitAndExpr");
    case BO_OrAssign:  NT("asgBitOrExpr");
    case BO_XorAssign: NT("asgBitXorExpr");
    }
  }
  const UnaryOperator *UO = dyn_cast<const UnaryOperator>(S);
  if (UO) {
    // XcodeML-C-0.9J.pdf 7.2(varAddr), 7.3(pointerRef), 7.8, 7.11
    QualType T = UO->getType();
    switch (UO->getOpcode()) {
    case UO_PostInc:   NT("postIncrExpr");
    case UO_PostDec:   NT("postDecrExpr");
    case UO_PreInc:    NT("preIncrExpr");
    case UO_PreDec:    NT("preDecrExpr");
    case UO_AddrOf:    NT("varAddr");
    case UO_Deref:     NT("pointerRef");
    case UO_Plus:      NT("UNDEF_UO_Plus");
    case UO_Minus:     NT("unaryMinusExpr");
    case UO_Not:       NT("bitNotExpr");
    case UO_LNot:      NT("logNotExpr");
    case UO_Real:      NT("UNDEF_UO_Real");
    case UO_Imag:      NT("UNDEF_UO_Imag");
    case UO_Extension: NT("UNDEF_UO_Extension");
    }
  }

  switch (S->getStmtClass()) {
  case Stmt::NoStmtClass:     NS("Stmt_NoStmtClass");
  case Stmt::GCCAsmStmtClass: NS("Stmt_GCCAsmStmtClass");
  case Stmt::MSAsmStmtClass:  NS("Stmt_MSAsmStmtClass");
  case Stmt::AttributedStmtClass: NS("Stmt_AttributedStmtClass");
  case Stmt::BreakStmtClass: NS("breakStatement"); //6.7
  case Stmt::CXXCatchStmtClass: NS("Stmt_CXXCatchStmtClass");
  case Stmt::CXXForRangeStmtClass: NS("Stmt_CXXForRangeStmtClass");
  case Stmt::CXXTryStmtClass: NS("Stmt_CXXTryStmtClass");
  case Stmt::CapturedStmtClass: NS("Stmt_CapturedStmtClass");
  case Stmt::CompoundStmtClass: {
    // 6.2
    newChild("compoundStatement");
    SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
    SV.TraverseChildOfStmt(S);
    WrapCompoundStatementBody(curNode, true);
    N("declarations");
  }
  case Stmt::ContinueStmtClass:
    //6.8
    NS("continueStatement");
  case Stmt::DeclStmtClass:
    return true; // everything is performed by WrapCompoundStatementBody
  case Stmt::DoStmtClass:
    //6.5
    WrapChild("+body", "condition");
    NS("doStatement");
  case Stmt::BinaryConditionalOperatorClass:
    //7.13
    NE("condExpr");
  case Stmt::ConditionalOperatorClass:
    //7.13
    NE("condExpr");
  case Stmt::AddrLabelExprClass: NS("Stmt_AddrLabelExprClass");
  case Stmt::ArraySubscriptExprClass: NS("Stmt_ArraySubscriptExprClass");
  case Stmt::ArrayTypeTraitExprClass: NS("Stmt_ArrayTypeTraitExprClass");
  case Stmt::AsTypeExprClass: NS("Stmt_AsTypeExprClass");
  case Stmt::AtomicExprClass: NS("Stmt_AtomicExprClass");
  case Stmt::BinaryOperatorClass: NS("Stmt_BinaryOperatorClass");
  case Stmt::CompoundAssignOperatorClass: NS("Stmt_CompoundAssignOperatorClass");
  case Stmt::BlockExprClass: NS("Stmt_BlockExprClass");
  case Stmt::CXXBindTemporaryExprClass: NS("Stmt_CXXBindTemporaryExprClass");
  case Stmt::CXXBoolLiteralExprClass: NS("Stmt_CXXBoolLiteralExprClass");
  case Stmt::CXXConstructExprClass: NS("Stmt_CXXConstructExprClass");
  case Stmt::CXXTemporaryObjectExprClass: NS("Stmt_CXXTemporaryObjectExprClass");
  case Stmt::CXXDefaultArgExprClass: NS("Stmt_CXXDefaultArgExprClass");
  case Stmt::CXXDefaultInitExprClass: NS("Stmt_CXXDefaultInitExprClass");
  case Stmt::CXXDeleteExprClass: NS("Stmt_CXXDeleteExprClass");
  case Stmt::CXXDependentScopeMemberExprClass: NS("Stmt_CXXDependentScopeMemberExprClass");
  case Stmt::CXXFoldExprClass: NS("Stmt_CXXFoldExprClass");
  case Stmt::CXXNewExprClass: NS("Stmt_CXXNewExprClass");
  case Stmt::CXXNoexceptExprClass: NS("Stmt_CXXNoexceptExprClass");
  case Stmt::CXXNullPtrLiteralExprClass: NS("Stmt_CXXNullPtrLiteralExprClass");
  case Stmt::CXXPseudoDestructorExprClass: NS("Stmt_CXXPseudoDestructorExprClass");
  case Stmt::CXXScalarValueInitExprClass: NS("Stmt_CXXScalarValueInitExprClass");
  case Stmt::CXXStdInitializerListExprClass: NS("Stmt_CXXStdInitializerListExprClass");
  case Stmt::CXXThisExprClass: NS("Stmt_CXXThisExprClass");
  case Stmt::CXXThrowExprClass: NS("Stmt_CXXThrowExprClass");
  case Stmt::CXXTypeidExprClass: NS("Stmt_CXXTypeidExprClass");
  case Stmt::CXXUnresolvedConstructExprClass: NS("Stmt_CXXUnresolvedConstructExprClass");
  case Stmt::CXXUuidofExprClass: NS("Stmt_CXXUuidofExprClass");
  case Stmt::CallExprClass:
    //7.9
    optContext.nameForDeclRefExpr = "function";
    HooksForStmt.push_back([this](Stmt *S){
        optContext.nameForDeclRefExpr = nullptr;
        newChild("arguments");
        return TraverseStmt(S);
      });
    HooksForStmt.push_back([this](Stmt *S){return TraverseStmt(S);});
    newChild("functionCall");
    return true;
  case Stmt::CUDAKernelCallExprClass: NS("Stmt_CUDAKernelCallExprClass");
  case Stmt::CXXMemberCallExprClass: NS("Stmt_CXXMemberCallExprClass");
  case Stmt::CXXOperatorCallExprClass: NS("Stmt_CXXOperatorCallExprClass");
  case Stmt::UserDefinedLiteralClass: NS("Stmt_UserDefinedLiteralClass");
  case Stmt::CStyleCastExprClass: NS("castExpr"); //7.12
  case Stmt::CXXFunctionalCastExprClass: NS("Stmt_CXXFunctionalCastExprClass");
  case Stmt::CXXConstCastExprClass: NS("Stmt_CXXConstCastExprClass");
  case Stmt::CXXDynamicCastExprClass: NS("Stmt_CXXDynamicCastExprClass");
  case Stmt::CXXReinterpretCastExprClass: NS("Stmt_CXXReinterpretCastExprClass");
  case Stmt::CXXStaticCastExprClass: NS("Stmt_CXXStaticCastExprClass");
  case Stmt::ObjCBridgedCastExprClass: NS("Stmt_ObjCBridgedCastExprClass");
  case Stmt::ImplicitCastExprClass:
    //NS("Stmt_ImplicitCastExprClass");
    /// XXX : experimental
    newComment("Stmt_ImplicitCastExprClass");
    if (!optContext.nameForDeclRefExpr) {
      optContext.nameForDeclRefExpr = "var";
    }
    return true;
  case Stmt::CharacterLiteralClass: NS("Stmt_CharacterLiteralClass");
  case Stmt::ChooseExprClass: NS("Stmt_ChooseExprClass");
  case Stmt::CompoundLiteralExprClass: NS("Stmt_CompoundLiteralExprClass");
  case Stmt::ConvertVectorExprClass: NS("Stmt_ConvertVectorExprClass");
  case Stmt::DeclRefExprClass:
    if (optContext.nameForDeclRefExpr) {
      NameChild(optContext.nameForDeclRefExpr);
    } else {
      NameChild("varAddr");
    }
    newComment("Stmt_DeclRefExprClass");
    return true;
  case Stmt::DependentScopeDeclRefExprClass: NS("Stmt_DependentScopeDeclRefExprClass");
  case Stmt::DesignatedInitExprClass: NS("Stmt_DesignatedInitExprClass");
  case Stmt::ExprWithCleanupsClass: NS("Stmt_ExprWithCleanupsClass");
  case Stmt::ExpressionTraitExprClass: NS("Stmt_ExpressionTraitExprClass");
  case Stmt::ExtVectorElementExprClass: NS("Stmt_ExtVectorElementExprClass");
  case Stmt::FloatingLiteralClass: {
    //7.1
#if 0
    double Value = static_cast<FloatingLiteral*>(S)->getValueAsApproximateDouble();
    raw_string_ostream OS(contentString);

    OS << Value;
    OS.str();
#else
    setContentBySource(S->getLocStart(), S->getLocEnd());
#endif
    NE("floatConstant");
  }
  case Stmt::FunctionParmPackExprClass: NS("Stmt_FunctionParmPackExprClass");
  case Stmt::GNUNullExprClass: NS("Stmt_GNUNullExprClass");
  case Stmt::GenericSelectionExprClass: NS("Stmt_GenericSelectionExprClass");
  case Stmt::ImaginaryLiteralClass: NS("Stmt_ImaginaryLiteralClass");
  case Stmt::ImplicitValueInitExprClass: NS("Stmt_ImplicitValueInitExprClass");
  case Stmt::InitListExprClass: NS("Stmt_InitListExprClass");
  case Stmt::IntegerLiteralClass: {
    //7.1 XXX: long long should be treated specially
    APInt Value = static_cast<IntegerLiteral*>(S)->getValue();
    raw_string_ostream OS(contentString);
    OS << *Value.getRawData();
    OS.str();
    NE("intConstant");
  }
  case Stmt::LambdaExprClass: NS("Stmt_LambdaExprClass");
  case Stmt::MSPropertyRefExprClass: NS("Stmt_MSPropertyRefExprClass");
  case Stmt::MaterializeTemporaryExprClass: NS("Stmt_MaterializeTemporaryExprClass");
  case Stmt::MemberExprClass:
    //7.5
    PropChild("member");
    optContext.nameForDeclRefExpr = nullptr;
    NE("memberRef"); 
  case Stmt::ObjCArrayLiteralClass: NS("Stmt_ObjCArrayLiteralClass");
  case Stmt::ObjCBoolLiteralExprClass: NS("Stmt_ObjCBoolLiteralExprClass");
  case Stmt::ObjCBoxedExprClass: NS("Stmt_ObjCBoxedExprClass");
  case Stmt::ObjCDictionaryLiteralClass: NS("Stmt_ObjCDictionaryLiteralClass");
  case Stmt::ObjCEncodeExprClass: NS("Stmt_ObjCEncodeExprClass");
  case Stmt::ObjCIndirectCopyRestoreExprClass: NS("Stmt_ObjCIndirectCopyRestoreExprClass");
  case Stmt::ObjCIsaExprClass: NS("Stmt_ObjCIsaExprClass");
  case Stmt::ObjCIvarRefExprClass: NS("Stmt_ObjCIvarRefExprClass");
  case Stmt::ObjCMessageExprClass: NS("Stmt_ObjCMessageExprClass");
  case Stmt::ObjCPropertyRefExprClass: NS("Stmt_ObjCPropertyRefExprClass");
  case Stmt::ObjCProtocolExprClass: NS("Stmt_ObjCProtocolExprClass");
  case Stmt::ObjCSelectorExprClass: NS("Stmt_ObjCSelectorExprClass");
  case Stmt::ObjCStringLiteralClass: NS("Stmt_ObjCStringLiteralClass");
  case Stmt::ObjCSubscriptRefExprClass: NS("Stmt_ObjCSubscriptRefExprClass");
  case Stmt::OffsetOfExprClass: NS("Stmt_OffsetOfExprClass");
  case Stmt::OpaqueValueExprClass: NS("Stmt_OpaqueValueExprClass");
  case Stmt::UnresolvedLookupExprClass: NS("Stmt_UnresolvedLookupExprClass");
  case Stmt::UnresolvedMemberExprClass: NS("Stmt_UnresolvedMemberExprClass");
  case Stmt::PackExpansionExprClass: NS("Stmt_PackExpansionExprClass");
  case Stmt::ParenExprClass: return true;; // no explicit node
  case Stmt::ParenListExprClass: NS("Stmt_ParenListExprClass");
  case Stmt::PredefinedExprClass: NS("Stmt_PredefinedExprClass");
  case Stmt::PseudoObjectExprClass: NS("Stmt_PseudoObjectExprClass");
  case Stmt::ShuffleVectorExprClass: NS("Stmt_ShuffleVectorExprClass");
  case Stmt::SizeOfPackExprClass: NS("Stmt_SizeOfPackExprClass");
  case Stmt::StmtExprClass: NS("Stmt_StmtExprClass");
  case Stmt::StringLiteralClass: {
    //7.1
    StringRef Data = static_cast<StringLiteral*>(S)->getString();
    raw_string_ostream OS(contentString);

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
    NE("stringConstant");
  }
  case Stmt::SubstNonTypeTemplateParmExprClass: NS("Stmt_SubstNonTypeTemplateParmExprClass");
  case Stmt::SubstNonTypeTemplateParmPackExprClass: NS("Stmt_SubstNonTypeTemplateParmPackExprClass");
  case Stmt::TypeTraitExprClass: NS("Stmt_TypeTraitExprClass");
  case Stmt::TypoExprClass: NS("Stmt_TypoExprClass");
  case Stmt::UnaryExprOrTypeTraitExprClass: NS("Stmt_UnaryExprOrTypeTraitExprClass");
  case Stmt::UnaryOperatorClass: NS("Stmt_UnaryOperatorClass");
  case Stmt::VAArgExprClass: NS("Stmt_VAArgExprClass");
  case Stmt::ForStmtClass:
    //6.6
    WrapChild("init", "condition", "iter", "+body");
    NS("forStatement");
  case Stmt::GotoStmtClass: NS("gotoStatement"); //6.10 XXX
  case Stmt::IfStmtClass:
    //6.3
    WrapChild("condition", "+then", "+else");
    NS("ifStatement");
  case Stmt::IndirectGotoStmtClass: NS("Stmt_IndirectGotoStmtClass");
  case Stmt::LabelStmtClass:
    //6.11
    WrapChild("name");
    NS("statementLabel");
  case Stmt::MSDependentExistsStmtClass: NS("Stmt_MSDependentExistsStmtClass");
  case Stmt::NullStmtClass: NS("Stmt_NullStmtClass");
  case Stmt::OMPAtomicDirectiveClass: NS("Stmt_OMPAtomicDirectiveClass");
  case Stmt::OMPBarrierDirectiveClass: NS("Stmt_OMPBarrierDirectiveClass");
  case Stmt::OMPCriticalDirectiveClass: NS("Stmt_OMPCriticalDirectiveClass");
  case Stmt::OMPFlushDirectiveClass: NS("Stmt_OMPFlushDirectiveClass");
  case Stmt::OMPForDirectiveClass: NS("Stmt_OMPForDirectiveClass");
  case Stmt::OMPForSimdDirectiveClass: NS("Stmt_OMPForSimdDirectiveClass");
  case Stmt::OMPParallelForDirectiveClass: NS("Stmt_OMPParallelForDirectiveClass");
  case Stmt::OMPParallelForSimdDirectiveClass: NS("Stmt_OMPParallelForSimdDirectiveClass");
  case Stmt::OMPSimdDirectiveClass: NS("Stmt_OMPSimdDirectiveClass");
  case Stmt::OMPMasterDirectiveClass: NS("Stmt_OMPMasterDirectiveClass");
  case Stmt::OMPOrderedDirectiveClass: NS("Stmt_OMPOrderedDirectiveClass");
  case Stmt::OMPParallelDirectiveClass: NS("Stmt_OMPParallelDirectiveClass");
  case Stmt::OMPParallelSectionsDirectiveClass: NS("Stmt_OMPParallelSectionsDirectiveClass");
  case Stmt::OMPSectionDirectiveClass: NS("Stmt_OMPSectionDirectiveClass");
  case Stmt::OMPSectionsDirectiveClass: NS("Stmt_OMPSectionsDirectiveClass");
  case Stmt::OMPSingleDirectiveClass: NS("Stmt_OMPSingleDirectiveClass");
  case Stmt::OMPTargetDirectiveClass: NS("Stmt_OMPTargetDirectiveClass");
  case Stmt::OMPTaskDirectiveClass: NS("Stmt_OMPTaskDirectiveClass");
  case Stmt::OMPTaskwaitDirectiveClass: NS("Stmt_OMPTaskwaitDirectiveClass");
  case Stmt::OMPTaskyieldDirectiveClass: NS("Stmt_OMPTaskyieldDirectiveClass");
  case Stmt::OMPTeamsDirectiveClass: NS("Stmt_OMPTeamsDirectiveClass");
  case Stmt::ObjCAtCatchStmtClass: NS("Stmt_ObjCAtCatchStmtClass");
  case Stmt::ObjCAtFinallyStmtClass: NS("Stmt_ObjCAtFinallyStmtClass");
  case Stmt::ObjCAtSynchronizedStmtClass: NS("Stmt_ObjCAtSynchronizedStmtClass");
  case Stmt::ObjCAtThrowStmtClass: NS("Stmt_ObjCAtThrowStmtClass");
  case Stmt::ObjCAtTryStmtClass: NS("Stmt_ObjCAtTryStmtClass");
  case Stmt::ObjCAutoreleasePoolStmtClass: NS("Stmt_ObjCAutoreleasePoolStmtClass");
  case Stmt::ObjCForCollectionStmtClass: NS("Stmt_ObjCForCollectionStmtClass");
  case Stmt::ReturnStmtClass:
    //6.9
    NS("returnStatement");
  case Stmt::SEHExceptStmtClass: NS("Stmt_SEHExceptStmtClass");
  case Stmt::SEHFinallyStmtClass: NS("Stmt_SEHFinallyStmtClass");
  case Stmt::SEHLeaveStmtClass: NS("Stmt_SEHLeaveStmtClass");
  case Stmt::SEHTryStmtClass: NS("Stmt_SEHTryStmtClass");
  case Stmt::CaseStmtClass:
    //6.13
    WrapChild("value");
    NS("caseLabel");
  case Stmt::DefaultStmtClass:
    //6.15
    NS("defaultLabel");
  case Stmt::SwitchStmtClass:
    //6.12
    WrapChild("value", "+body");
    NS("switchStatement");
  case Stmt::WhileStmtClass:
    //6.4
    WrapChild("condition", "+body");
    NS("whileStatement");
  }
}

bool
DeclarationsVisitor::PreVisitType(QualType T) {
  if (T.isNull()) {
    newComment("Type_NULL");
    return false;
  }
  const Type *Tptr = T.getTypePtrOrNull();

  // XXX experimental code
  raw_string_ostream OS(contentString);
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
}

bool
DeclarationsVisitor::PreVisitTypeLoc(TypeLoc TL) {
  if (TL.isNull()) {
    newComment("TypeLoc_NULL");
    return false;
  }

  switch (TL.getTypeLocClass()) {
  case TypeLoc::Qualified: NTypeLoc("TypeLoc_Qualified");
  case TypeLoc::Builtin: NTypeLoc("TypeLoc_Builtin");
  case TypeLoc::Complex: NTypeLoc("TypeLoc_Complex");
  case TypeLoc::Pointer: NTypeLoc("TypeLoc_Pointer");
  case TypeLoc::BlockPointer: NTypeLoc("TypeLoc_BlockPointer");
  case TypeLoc::LValueReference: NTypeLoc("TypeLoc_LValueReference");
  case TypeLoc::RValueReference: NTypeLoc("TypeLoc_RValueReference");
  case TypeLoc::MemberPointer: NTypeLoc("TypeLoc_MemberPointer");
  case TypeLoc::ConstantArray: NTypeLoc("TypeLoc_ConstantArray");
  case TypeLoc::IncompleteArray: NTypeLoc("TypeLoc_IncompleteArray");
  case TypeLoc::VariableArray: NTypeLoc("TypeLoc_VariableArray");
  case TypeLoc::DependentSizedArray: NTypeLoc("TypeLoc_DependentSizedArray");
  case TypeLoc::DependentSizedExtVector: NTypeLoc("TypeLoc_DependentSizedExtVector");
  case TypeLoc::Vector: NTypeLoc("TypeLoc_Vector");
  case TypeLoc::ExtVector: NTypeLoc("TypeLoc_ExtVector");
  case TypeLoc::FunctionProto: NTypeLoc("TypeLoc_FunctionProto");
  case TypeLoc::FunctionNoProto: NTypeLoc("TypeLoc_FunctionNoProto");
  case TypeLoc::UnresolvedUsing: NTypeLoc("TypeLoc_UnresolvedUsing");
  case TypeLoc::Paren: NTypeLoc("TypeLoc_Paren");
  case TypeLoc::Typedef: NTypeLoc("TypeLoc_Typedef");
  case TypeLoc::Adjusted: NTypeLoc("TypeLoc_Adjusted");
  case TypeLoc::Decayed: NTypeLoc("TypeLoc_Decayed");
  case TypeLoc::TypeOfExpr: NTypeLoc("TypeLoc_TypeOfExpr");
  case TypeLoc::TypeOf: NTypeLoc("TypeLoc_TypeOf");
  case TypeLoc::Decltype: NTypeLoc("TypeLoc_Decltype");
  case TypeLoc::UnaryTransform: NTypeLoc("TypeLoc_UnaryTransform");
  case TypeLoc::Record: NTypeLoc("TypeLoc_Record");
  case TypeLoc::Enum: NTypeLoc("TypeLoc_Enum");
  case TypeLoc::Elaborated: NTypeLoc("TypeLoc_Elaborated");
  case TypeLoc::Attributed: NTypeLoc("TypeLoc_Attributed");
  case TypeLoc::TemplateTypeParm: NTypeLoc("TypeLoc_TemplateTypeParm");
  case TypeLoc::SubstTemplateTypeParm: NTypeLoc("TypeLoc_SubstTemplateTypeParm");
  case TypeLoc::SubstTemplateTypeParmPack: NTypeLoc("TypeLoc_SubstTemplateTypeParmPack");
  case TypeLoc::TemplateSpecialization: NTypeLoc("TypeLoc_TemplateSpecialization");
  case TypeLoc::Auto: NTypeLoc("TypeLoc_Auto");
  case TypeLoc::InjectedClassName: NTypeLoc("TypeLoc_InjectedClassName");
  case TypeLoc::DependentName: NTypeLoc("TypeLoc_DependentName");
  case TypeLoc::DependentTemplateSpecialization: NTypeLoc("TypeLoc_DependentTemplateSpecialization");
  case TypeLoc::PackExpansion: NTypeLoc("TypeLoc_PackExpansion");
  case TypeLoc::ObjCObject: NTypeLoc("TypeLoc_ObjCObject");
  case TypeLoc::ObjCInterface: NTypeLoc("TypeLoc_ObjCInterface");
  case TypeLoc::ObjCObjectPointer: NTypeLoc("TypeLoc_ObjCObjectPointer");
  case TypeLoc::Atomic: NTypeLoc("TypeLoc_Atomic");
  }
}

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

  setContentBySource(A->getLocation(), A->getLocation());
  newProp("name", contentString.c_str());

  raw_string_ostream OS(contentString);
  ASTContext &CXT = mangleContext->getASTContext();
  A->printPretty(OS, PrintingPolicy(CXT.getLangOpts()));
  newComment(OS.str().c_str());

  return true;
}

bool
DeclarationsVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return false;
  }
  switch (D->getKind()) {
  case Decl::AccessSpec: ND("Decl_AccessSpec");
  case Decl::Block: ND("Decl_Block");
  case Decl::Captured: ND("Decl_Captured");
  case Decl::ClassScopeFunctionSpecialization: ND("Decl_ClassScopeFunctionSpecialization");
  case Decl::Empty: ND("Decl_Empty");
  case Decl::FileScopeAsm: ND("Decl_FileScopeAsm");
  case Decl::Friend: ND("Decl_Friend");
  case Decl::FriendTemplate: ND("Decl_FriendTemplate");
  case Decl::Import: ND("Decl_Import");
  case Decl::LinkageSpec: ND("Decl_LinkageSpec");
  case Decl::Label: ND("Decl_Label");
  case Decl::Namespace: ND("Decl_Namespace");
  case Decl::NamespaceAlias: ND("Decl_NamespaceAlias");
  case Decl::ObjCCompatibleAlias: ND("Decl_ObjCCompatibleAlias");
  case Decl::ObjCCategory: ND("Decl_ObjCCategory");
  case Decl::ObjCCategoryImpl: ND("Decl_ObjCCategoryImpl");
  case Decl::ObjCImplementation: ND("Decl_ObjCImplementation");
  case Decl::ObjCInterface: ND("Decl_ObjCInterface");
  case Decl::ObjCProtocol: ND("Decl_ObjCProtocol");
  case Decl::ObjCMethod: ND("Decl_ObjCMethod");
  case Decl::ObjCProperty: ND("Decl_ObjCProperty");
  case Decl::ClassTemplate: ND("Decl_ClassTemplate");
  case Decl::FunctionTemplate: ND("Decl_FunctionTemplate");
  case Decl::TypeAliasTemplate: ND("Decl_TypeAliasTemplate");
  case Decl::VarTemplate: ND("Decl_VarTemplate");
  case Decl::TemplateTemplateParm: ND("Decl_TemplateTemplateParm");
  case Decl::Enum: ND("Decl_Enum");
  case Decl::Record: ND("Decl_Record");
  case Decl::CXXRecord: ND("Decl_CXXRecord");
  case Decl::ClassTemplateSpecialization: ND("Decl_ClassTemplateSpecialization");
  case Decl::ClassTemplatePartialSpecialization: ND("Decl_ClassTemplatePartialSpecialization");
  case Decl::TemplateTypeParm: ND("Decl_TemplateTypeParm");
  case Decl::TypeAlias: ND("Decl_TypeAlias");
  case Decl::Typedef: ND("Decl_Typedef");
  case Decl::UnresolvedUsingTypename: ND("Decl_UnresolvedUsingTypename");
  case Decl::Using: ND("Decl_Using");
  case Decl::UsingDirective: ND("Decl_UsingDirective");
  case Decl::UsingShadow: ND("Decl_UsingShadow");
  case Decl::Field: ND("Decl_Field");
  case Decl::ObjCAtDefsField: ND("Decl_ObjCAtDefsField");
  case Decl::ObjCIvar: ND("Decl_ObjCIvar");
  case Decl::Function: ND("Decl_Function");
  case Decl::CXXMethod: ND("Decl_CXXMethod");
  case Decl::CXXConstructor: ND("Decl_CXXConstructor");
  case Decl::CXXConversion: ND("Decl_CXXConversion");
  case Decl::CXXDestructor: ND("Decl_CXXDestructor");
  case Decl::MSProperty: ND("Decl_MSProperty");
  case Decl::NonTypeTemplateParm: ND("Decl_NonTypeTemplateParm");
  case Decl::Var: ND("Decl_Var");
  case Decl::ImplicitParam: ND("Decl_ImplicitParam");
  case Decl::ParmVar: ND("Decl_ParmVar");
  case Decl::VarTemplateSpecialization: ND("Decl_VarTemplateSpecialization");
  case Decl::VarTemplatePartialSpecialization: ND("Decl_VarTemplatePartialSpecialization");
  case Decl::EnumConstant: ND("Decl_EnumConstant");
  case Decl::IndirectField: ND("Decl_IndirectField");
  case Decl::UnresolvedUsingValue: ND("Decl_UnresolvedUsingValue");
  case Decl::OMPThreadPrivate: ND("Decl_OMPThreadPrivate");
  case Decl::ObjCPropertyImpl: ND("Decl_ObjCPropertyImpl");
  case Decl::StaticAssert: ND("Decl_StaticAssert");
  case Decl::TranslationUnit:
    if (OptDisableDeclarations) {
      return false; // stop traverse
    } else {
      return true; // no need to create a child
    }
  }
}

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

bool
DeclarationsVisitor::PreVisitNestedNameSpecifier(NestedNameSpecifier *NNS) {
  if (!NNS) {
    newComment("NestedNameSpecifier_NULL");
    return false;
  }
  switch (NNS->getKind()) {
  case NestedNameSpecifier::Identifier: N("NestedNameSpecifier_Identifier");
  case NestedNameSpecifier::Namespace: N("NestedNameSpecifier_Namespace");
  case NestedNameSpecifier::NamespaceAlias: N("NestedNameSpecifier_NamespaceAlias");
  case NestedNameSpecifier::Global: N("NestedNameSpecifier_Global");
  case NestedNameSpecifier::Super: N("NestedNameSpecifier_Super");
  case NestedNameSpecifier::TypeSpec: N("NestedNameSpecifier_TypeSpec");
  case NestedNameSpecifier::TypeSpecWithTemplate: N("NestedNameSpecifier_TypeSpecWithTemplate");
  }
}

bool
DeclarationsVisitor::PreVisitNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS) {
  if (!NNS) {
    newComment("NestedNameSpecifierLoc_NULL");
    return false;
  }
  switch (NNS.getNestedNameSpecifier()->getKind()) {
  case NestedNameSpecifier::Identifier: N("NestedNameSpecifierLoc_Identifier");
  case NestedNameSpecifier::Namespace: N("NestedNameSpecifierLoc_Namespace");
  case NestedNameSpecifier::NamespaceAlias: N("NestedNameSpecifierLoc_NamespaceAlias");
  case NestedNameSpecifier::Global: N("NestedNameSpecifierLoc_Global");
  case NestedNameSpecifier::Super: N("NestedNameSpecifierLoc_Super");
  case NestedNameSpecifier::TypeSpec: N("NestedNameSpecifierLoc_TypeSpec");
  case NestedNameSpecifier::TypeSpecWithTemplate: N("NestedNameSpecifierLoc_TypeSpecWithTemplate");
  }
}

bool
DeclarationsVisitor::PreVisitDeclarationNameInfo(DeclarationNameInfo NameInfo) {
  DeclarationName DN = NameInfo.getName();
  IdentifierInfo *II = DN.getAsIdentifierInfo();
  if (II) {
    contentString = II->getNameStart();
  }

  if (optContext.explicitname) {
    N(optContext.explicitname);
  }

  switch (NameInfo.getName().getNameKind()) {
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

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
