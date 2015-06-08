#include "XcodeMlVisitorBase.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/Signals.h"

#include <time.h>
#include <string>

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

cl::OptionCategory C2XcodeMLCategory("CtoXcodeML options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static std::unique_ptr<opt::OptTable> Options(createDriverOptTable());

static cl::opt<bool>
OptTraceXTTV("trace-xttv",
             cl::desc("emit traces on XcodeMlTypeTableVisitor"),
             cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptTraceXSV("trace-xsv",
            cl::desc("emit traces on XcodeMlSymbolVisitor"),
            cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptTraceXDV("trace-xdv",
            cl::desc("emit traces on XcodeMlDeclarationsVisitor"),
            cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXTTV("disable-xttv",
               cl::desc("disable XcodeMlTypeTableVisitor"),
               cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXSV("disable-xsv",
              cl::desc("disable XcodeMlSymbolsVisitor"),
              cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXDV("disable-xdv",
              cl::desc("disable XcodeMlDeclarationsVisitor"),
              cl::cat(C2XcodeMLCategory));


class XcodeMlTypeTableVisitor
    : public XcodeMlVisitorBase<XcodeMlTypeTableVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override {
        return OptTraceXTTV ? "XTTV" : nullptr;
    }
    bool PreVisitStmt(const Stmt *S) {
        // do not create a new child
        return true;
    }
    bool PreVisitDecl(const Decl *D) {
        // do not create a new child
        return true;
    }
};
class XcodeMlSymbolsVisitor
    : public XcodeMlVisitorBase<XcodeMlSymbolsVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase<XcodeMlSymbolsVisitor>::XcodeMlVisitorBase;

    const char *getVisitorName() const override {
        return OptTraceXSV ? "XSV" : nullptr;
    }
};

class XcodeMlDeclarationsVisitor
    : public XcodeMlVisitorBase<XcodeMlDeclarationsVisitor> {
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const override {
        return OptTraceXDV ? "XDV" : nullptr;
    }

    const char *NameForStmt(const Stmt *S) const {
        if (!S) {
            return "Stmt_UNKNOWN";
        }
        const BinaryOperator *BO = dyn_cast<const BinaryOperator>(S);
        if (BO) {
            // XcodeML-C-0.9J.pdf: 7.6(assignExpr), 7.7, 7.10(commmaExpr)
            switch (BO->getOpcode()) {
            case BO_PtrMemD:   return "UNDEF_BO_PtrMemD";
            case BO_PtrMemI:   return "UNDEF_BO_PtrMemI";
            case BO_Mul:       return "mulExpr";
            case BO_Div:       return "divExpr";
            case BO_Rem:       return "modExpr";
            case BO_Add:       return "plusExpr";
            case BO_Sub:       return "minusExpr";
            case BO_Shl:       return "LshiftExpr";
            case BO_Shr:       return "RshiftExpr";
            case BO_LT:        return "logLTExpr";
            case BO_GT:        return "logGTExpr";
            case BO_LE:        return "logLEExpr";
            case BO_GE:        return "logGEExpr";
            case BO_EQ:        return "logEQExpr";
            case BO_NE:        return "logNEQExpr";
            case BO_And:       return "bitAndExpr";
            case BO_Xor:       return "bitXorExpr";
            case BO_Or:        return "bitOrExpr";
            case BO_LAnd:      return "logAndExpr";
            case BO_LOr:       return "logOrExpr";
            case BO_Assign:    return "assignExpr";
            case BO_Comma:     return "commaExpr";
            case BO_MulAssign: return "asgMulExpr";
            case BO_DivAssign: return "asgDivExpr";
            case BO_RemAssign: return "asgModExpr";
            case BO_AddAssign: return "asgPlusExpr";
            case BO_SubAssign: return "asgMinusExpr";
            case BO_ShlAssign: return "asgLshiftExpr";
            case BO_ShrAssign: return "asgRshiftExpr";
            case BO_AndAssign: return "asgBitAndExpr";
            case BO_OrAssign:  return "asgBitOrExpr";
            case BO_XorAssign: return "asgBitXorExpr";
            }
        }
        const UnaryOperator *UO = dyn_cast<const UnaryOperator>(S);
        if (UO) {
            // XcodeML-C-0.9J.pdf 7.2(varAddr), 7.3(pointerRef), 7.8, 7.11
            switch (UO->getOpcode()) {
            case UO_PostInc:   return "postIncrExpr";
            case UO_PostDec:   return "postDecrExpr";
            case UO_PreInc:    return "preIncrExpr";
            case UO_PreDec:    return "preDecrExpr";
            case UO_AddrOf:    return "varAddr";
            case UO_Deref:     return "pointerRef";
            case UO_Plus:      return "UNDEF_UO_Plus";
            case UO_Minus:     return "unaryMinusExpr";
            case UO_Not:       return "bitNotExpr";
            case UO_LNot:      return "logNotExpr";
            case UO_Real:      return "UNDEF_UO_Real";
            case UO_Imag:      return "UNDEF_UO_Imag";
            case UO_Extension: return "UNDEF_UO_Extension";
            }
        }
        switch (S->getStmtClass()) {
        case Stmt::NoStmtClass:     return "Stmt_NoStmtClass";
        case Stmt::GCCAsmStmtClass: return "Stmt_GCCAsmStmtClass";
        case Stmt::MSAsmStmtClass:  return "Stmt_MSAsmStmtClass";
        case Stmt::AttributedStmtClass: return "Stmt_AttributedStmtClass";
        case Stmt::BreakStmtClass: return "Stmt_BreakStmtClass";
        case Stmt::CXXCatchStmtClass: return "Stmt_CXXCatchStmtClass";
        case Stmt::CXXForRangeStmtClass: return "Stmt_CXXForRangeStmtClass";
        case Stmt::CXXTryStmtClass: return "Stmt_CXXTryStmtClass";
        case Stmt::CapturedStmtClass: return "Stmt_CapturedStmtClass";
        case Stmt::CompoundStmtClass: return "Stmt_CompoundStmtClass";
        case Stmt::ContinueStmtClass: return "Stmt_ContinueStmtClass";
        case Stmt::DeclStmtClass: return "Stmt_DeclStmtClass";
        case Stmt::DoStmtClass: return "Stmt_DoStmtClass";
        case Stmt::BinaryConditionalOperatorClass: return "condExpr"; //7.13
        case Stmt::ConditionalOperatorClass: return "condExpr"; //7.13
        case Stmt::AddrLabelExprClass: return "Stmt_AddrLabelExprClass";
        case Stmt::ArraySubscriptExprClass: return "Stmt_ArraySubscriptExprClass";
        case Stmt::ArrayTypeTraitExprClass: return "Stmt_ArrayTypeTraitExprClass";
        case Stmt::AsTypeExprClass: return "Stmt_AsTypeExprClass";
        case Stmt::AtomicExprClass: return "Stmt_AtomicExprClass";
        case Stmt::BinaryOperatorClass: return "Stmt_BinaryOperatorClass";
        case Stmt::CompoundAssignOperatorClass: return "Stmt_CompoundAssignOperatorClass";
        case Stmt::BlockExprClass: return "Stmt_BlockExprClass";
        case Stmt::CXXBindTemporaryExprClass: return "Stmt_CXXBindTemporaryExprClass";
        case Stmt::CXXBoolLiteralExprClass: return "Stmt_CXXBoolLiteralExprClass";
        case Stmt::CXXConstructExprClass: return "Stmt_CXXConstructExprClass";
        case Stmt::CXXTemporaryObjectExprClass: return "Stmt_CXXTemporaryObjectExprClass";
        case Stmt::CXXDefaultArgExprClass: return "Stmt_CXXDefaultArgExprClass";
        case Stmt::CXXDefaultInitExprClass: return "Stmt_CXXDefaultInitExprClass";
        case Stmt::CXXDeleteExprClass: return "Stmt_CXXDeleteExprClass";
        case Stmt::CXXDependentScopeMemberExprClass: return "Stmt_CXXDependentScopeMemberExprClass";
        case Stmt::CXXFoldExprClass: return "Stmt_CXXFoldExprClass";
        case Stmt::CXXNewExprClass: return "Stmt_CXXNewExprClass";
        case Stmt::CXXNoexceptExprClass: return "Stmt_CXXNoexceptExprClass";
        case Stmt::CXXNullPtrLiteralExprClass: return "Stmt_CXXNullPtrLiteralExprClass";
        case Stmt::CXXPseudoDestructorExprClass: return "Stmt_CXXPseudoDestructorExprClass";
        case Stmt::CXXScalarValueInitExprClass: return "Stmt_CXXScalarValueInitExprClass";
        case Stmt::CXXStdInitializerListExprClass: return "Stmt_CXXStdInitializerListExprClass";
        case Stmt::CXXThisExprClass: return "Stmt_CXXThisExprClass";
        case Stmt::CXXThrowExprClass: return "Stmt_CXXThrowExprClass";
        case Stmt::CXXTypeidExprClass: return "Stmt_CXXTypeidExprClass";
        case Stmt::CXXUnresolvedConstructExprClass: return "Stmt_CXXUnresolvedConstructExprClass";
        case Stmt::CXXUuidofExprClass: return "Stmt_CXXUuidofExprClass";
        case Stmt::CallExprClass: return "Stmt_CallExprClass";
        case Stmt::CUDAKernelCallExprClass: return "Stmt_CUDAKernelCallExprClass";
        case Stmt::CXXMemberCallExprClass: return "Stmt_CXXMemberCallExprClass";
        case Stmt::CXXOperatorCallExprClass: return "Stmt_CXXOperatorCallExprClass";
        case Stmt::UserDefinedLiteralClass: return "Stmt_UserDefinedLiteralClass";
        case Stmt::CStyleCastExprClass: return "Stmt_CStyleCastExprClass";
        case Stmt::CXXFunctionalCastExprClass: return "Stmt_CXXFunctionalCastExprClass";
        case Stmt::CXXConstCastExprClass: return "Stmt_CXXConstCastExprClass";
        case Stmt::CXXDynamicCastExprClass: return "Stmt_CXXDynamicCastExprClass";
        case Stmt::CXXReinterpretCastExprClass: return "Stmt_CXXReinterpretCastExprClass";
        case Stmt::CXXStaticCastExprClass: return "Stmt_CXXStaticCastExprClass";
        case Stmt::ObjCBridgedCastExprClass: return "Stmt_ObjCBridgedCastExprClass";
        case Stmt::ImplicitCastExprClass: return "Stmt_ImplicitCastExprClass";
        case Stmt::CharacterLiteralClass: return "Stmt_CharacterLiteralClass";
        case Stmt::ChooseExprClass: return "Stmt_ChooseExprClass";
        case Stmt::CompoundLiteralExprClass: return "Stmt_CompoundLiteralExprClass";
        case Stmt::ConvertVectorExprClass: return "Stmt_ConvertVectorExprClass";
        case Stmt::DeclRefExprClass: return "Stmt_DeclRefExprClass";
        case Stmt::DependentScopeDeclRefExprClass: return "Stmt_DependentScopeDeclRefExprClass";
        case Stmt::DesignatedInitExprClass: return "Stmt_DesignatedInitExprClass";
        case Stmt::ExprWithCleanupsClass: return "Stmt_ExprWithCleanupsClass";
        case Stmt::ExpressionTraitExprClass: return "Stmt_ExpressionTraitExprClass";
        case Stmt::ExtVectorElementExprClass: return "Stmt_ExtVectorElementExprClass";
        case Stmt::FloatingLiteralClass: return "Stmt_FloatingLiteralClass";
        case Stmt::FunctionParmPackExprClass: return "Stmt_FunctionParmPackExprClass";
        case Stmt::GNUNullExprClass: return "Stmt_GNUNullExprClass";
        case Stmt::GenericSelectionExprClass: return "Stmt_GenericSelectionExprClass";
        case Stmt::ImaginaryLiteralClass: return "Stmt_ImaginaryLiteralClass";
        case Stmt::ImplicitValueInitExprClass: return "Stmt_ImplicitValueInitExprClass";
        case Stmt::InitListExprClass: return "Stmt_InitListExprClass";
        case Stmt::IntegerLiteralClass: return "Stmt_IntegerLiteralClass";
        case Stmt::LambdaExprClass: return "Stmt_LambdaExprClass";
        case Stmt::MSPropertyRefExprClass: return "Stmt_MSPropertyRefExprClass";
        case Stmt::MaterializeTemporaryExprClass: return "Stmt_MaterializeTemporaryExprClass";
        case Stmt::MemberExprClass: return "Stmt_MemberExprClass";
        case Stmt::ObjCArrayLiteralClass: return "Stmt_ObjCArrayLiteralClass";
        case Stmt::ObjCBoolLiteralExprClass: return "Stmt_ObjCBoolLiteralExprClass";
        case Stmt::ObjCBoxedExprClass: return "Stmt_ObjCBoxedExprClass";
        case Stmt::ObjCDictionaryLiteralClass: return "Stmt_ObjCDictionaryLiteralClass";
        case Stmt::ObjCEncodeExprClass: return "Stmt_ObjCEncodeExprClass";
        case Stmt::ObjCIndirectCopyRestoreExprClass: return "Stmt_ObjCIndirectCopyRestoreExprClass";
        case Stmt::ObjCIsaExprClass: return "Stmt_ObjCIsaExprClass";
        case Stmt::ObjCIvarRefExprClass: return "Stmt_ObjCIvarRefExprClass";
        case Stmt::ObjCMessageExprClass: return "Stmt_ObjCMessageExprClass";
        case Stmt::ObjCPropertyRefExprClass: return "Stmt_ObjCPropertyRefExprClass";
        case Stmt::ObjCProtocolExprClass: return "Stmt_ObjCProtocolExprClass";
        case Stmt::ObjCSelectorExprClass: return "Stmt_ObjCSelectorExprClass";
        case Stmt::ObjCStringLiteralClass: return "Stmt_ObjCStringLiteralClass";
        case Stmt::ObjCSubscriptRefExprClass: return "Stmt_ObjCSubscriptRefExprClass";
        case Stmt::OffsetOfExprClass: return "Stmt_OffsetOfExprClass";
        case Stmt::OpaqueValueExprClass: return "Stmt_OpaqueValueExprClass";
        case Stmt::UnresolvedLookupExprClass: return "Stmt_UnresolvedLookupExprClass";
        case Stmt::UnresolvedMemberExprClass: return "Stmt_UnresolvedMemberExprClass";
        case Stmt::PackExpansionExprClass: return "Stmt_PackExpansionExprClass";
        case Stmt::ParenExprClass: return "Stmt_ParenExprClass";
        case Stmt::ParenListExprClass: return "Stmt_ParenListExprClass";
        case Stmt::PredefinedExprClass: return "Stmt_PredefinedExprClass";
        case Stmt::PseudoObjectExprClass: return "Stmt_PseudoObjectExprClass";
        case Stmt::ShuffleVectorExprClass: return "Stmt_ShuffleVectorExprClass";
        case Stmt::SizeOfPackExprClass: return "Stmt_SizeOfPackExprClass";
        case Stmt::StmtExprClass: return "Stmt_StmtExprClass";
        case Stmt::StringLiteralClass: return "Stmt_StringLiteralClass";
        case Stmt::SubstNonTypeTemplateParmExprClass: return "Stmt_SubstNonTypeTemplateParmExprClass";
        case Stmt::SubstNonTypeTemplateParmPackExprClass: return "Stmt_SubstNonTypeTemplateParmPackExprClass";
        case Stmt::TypeTraitExprClass: return "Stmt_TypeTraitExprClass";
        case Stmt::TypoExprClass: return "Stmt_TypoExprClass";
        case Stmt::UnaryExprOrTypeTraitExprClass: return "Stmt_UnaryExprOrTypeTraitExprClass";
        case Stmt::UnaryOperatorClass: return "Stmt_UnaryOperatorClass";
        case Stmt::VAArgExprClass: return "Stmt_VAArgExprClass";
        case Stmt::ForStmtClass: return "Stmt_ForStmtClass";
        case Stmt::GotoStmtClass: return "Stmt_GotoStmtClass";
        case Stmt::IfStmtClass: return "Stmt_IfStmtClass";
        case Stmt::IndirectGotoStmtClass: return "Stmt_IndirectGotoStmtClass";
        case Stmt::LabelStmtClass: return "Stmt_LabelStmtClass";
        case Stmt::MSDependentExistsStmtClass: return "Stmt_MSDependentExistsStmtClass";
        case Stmt::NullStmtClass: return "Stmt_NullStmtClass";
        case Stmt::OMPAtomicDirectiveClass: return "Stmt_OMPAtomicDirectiveClass";
        case Stmt::OMPBarrierDirectiveClass: return "Stmt_OMPBarrierDirectiveClass";
        case Stmt::OMPCriticalDirectiveClass: return "Stmt_OMPCriticalDirectiveClass";
        case Stmt::OMPFlushDirectiveClass: return "Stmt_OMPFlushDirectiveClass";
        case Stmt::OMPForDirectiveClass: return "Stmt_OMPForDirectiveClass";
        case Stmt::OMPForSimdDirectiveClass: return "Stmt_OMPForSimdDirectiveClass";
        case Stmt::OMPParallelForDirectiveClass: return "Stmt_OMPParallelForDirectiveClass";
        case Stmt::OMPParallelForSimdDirectiveClass: return "Stmt_OMPParallelForSimdDirectiveClass";
        case Stmt::OMPSimdDirectiveClass: return "Stmt_OMPSimdDirectiveClass";
        case Stmt::OMPMasterDirectiveClass: return "Stmt_OMPMasterDirectiveClass";
        case Stmt::OMPOrderedDirectiveClass: return "Stmt_OMPOrderedDirectiveClass";
        case Stmt::OMPParallelDirectiveClass: return "Stmt_OMPParallelDirectiveClass";
        case Stmt::OMPParallelSectionsDirectiveClass: return "Stmt_OMPParallelSectionsDirectiveClass";
        case Stmt::OMPSectionDirectiveClass: return "Stmt_OMPSectionDirectiveClass";
        case Stmt::OMPSectionsDirectiveClass: return "Stmt_OMPSectionsDirectiveClass";
        case Stmt::OMPSingleDirectiveClass: return "Stmt_OMPSingleDirectiveClass";
        case Stmt::OMPTargetDirectiveClass: return "Stmt_OMPTargetDirectiveClass";
        case Stmt::OMPTaskDirectiveClass: return "Stmt_OMPTaskDirectiveClass";
        case Stmt::OMPTaskwaitDirectiveClass: return "Stmt_OMPTaskwaitDirectiveClass";
        case Stmt::OMPTaskyieldDirectiveClass: return "Stmt_OMPTaskyieldDirectiveClass";
        case Stmt::OMPTeamsDirectiveClass: return "Stmt_OMPTeamsDirectiveClass";
        case Stmt::ObjCAtCatchStmtClass: return "Stmt_ObjCAtCatchStmtClass";
        case Stmt::ObjCAtFinallyStmtClass: return "Stmt_ObjCAtFinallyStmtClass";
        case Stmt::ObjCAtSynchronizedStmtClass: return "Stmt_ObjCAtSynchronizedStmtClass";
        case Stmt::ObjCAtThrowStmtClass: return "Stmt_ObjCAtThrowStmtClass";
        case Stmt::ObjCAtTryStmtClass: return "Stmt_ObjCAtTryStmtClass";
        case Stmt::ObjCAutoreleasePoolStmtClass: return "Stmt_ObjCAutoreleasePoolStmtClass";
        case Stmt::ObjCForCollectionStmtClass: return "Stmt_ObjCForCollectionStmtClass";
        case Stmt::ReturnStmtClass: return "Stmt_ReturnStmtClass";
        case Stmt::SEHExceptStmtClass: return "Stmt_SEHExceptStmtClass";
        case Stmt::SEHFinallyStmtClass: return "Stmt_SEHFinallyStmtClass";
        case Stmt::SEHLeaveStmtClass: return "Stmt_SEHLeaveStmtClass";
        case Stmt::SEHTryStmtClass: return "Stmt_SEHTryStmtClass";
        case Stmt::CaseStmtClass: return "Stmt_CaseStmtClass";
        case Stmt::DefaultStmtClass: return "Stmt_DefaultStmtClass";
        case Stmt::SwitchStmtClass: return "Stmt_SwitchStmtClass";
        case Stmt::WhileStmtClass: return "Stmt_WhileStmtClass";
        }
    }
};


class XcodeMlASTConsumer : public ASTConsumer {
    const xmlNodePtr rootNode;

public:
    explicit XcodeMlASTConsumer(xmlNodePtr N) : rootNode(N) {};

    virtual void HandleTranslationUnit(ASTContext &CXT) override {
        Decl *D = CXT.getTranslationUnitDecl();

        if (!OptDisableXTTV) {
            XcodeMlTypeTableVisitor TTV(CXT, rootNode, "TypeTable");
            TTV.BridgeDecl(D);
        }
        if (!OptDisableXSV) {
            XcodeMlSymbolsVisitor SV(CXT, rootNode, "Symbols");
            SV.BridgeDecl(D);
        }
        if (!OptDisableXDV) {
            XcodeMlDeclarationsVisitor DV(CXT, rootNode, "globalDeclarations");
            DV.BridgeDecl(D);
        }
    }
#if 0
    virtual bool HandleTopLevelDecl(DeclGroupRef DG) override {
        // We can check whether parsing should be continued or not
        // at the time that each declaration parsing is done.
        // default: true.
        return true;
    }
#endif
};

class XcodeMlASTDumpAction : public ASTFrontendAction {
private:
    xmlDocPtr xmlDoc;

public:
    bool BeginSourceFileAction(clang::CompilerInstance& CI,
                             StringRef Filename) override {
        xmlDoc = xmlNewDoc(BAD_CAST "1.0");
        xmlNodePtr rootnode
            = xmlNewNode(nullptr, BAD_CAST "XcodeProgram");
        xmlDocSetRootElement(xmlDoc, rootnode);

        char strftimebuf[BUFSIZ];
        time_t t = time(nullptr);

        strftime(strftimebuf, sizeof strftimebuf, "%F %T", localtime(&t));

        xmlNewProp(rootnode, BAD_CAST "source", BAD_CAST Filename.data());
        xmlNewProp(rootnode, BAD_CAST "language", BAD_CAST "C");
        xmlNewProp(rootnode, BAD_CAST "time", BAD_CAST strftimebuf);

        return true;
    };

    virtual std::unique_ptr<ASTConsumer>
    CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        std::unique_ptr<ASTConsumer>
            C(new XcodeMlASTConsumer(xmlDocGetRootElement(xmlDoc)));
        return C;
    }

    void EndSourceFileAction(void) override {
        xmlSaveFormatFileEnc("-", xmlDoc, "UTF-8", 1);
        xmlFreeDoc(xmlDoc);
    }
};

int main(int argc, const char **argv) {
    llvm::sys::PrintStackTraceOnErrorSignal();
    CommonOptionsParser OptionsParser(argc, argv, C2XcodeMLCategory);
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());
    Tool.appendArgumentsAdjuster(clang::tooling::getClangSyntaxOnlyAdjuster());

    std::unique_ptr<FrontendActionFactory> FrontendFactory
        = newFrontendActionFactory<XcodeMlASTDumpAction>();
    return Tool.run(FrontendFactory.get());
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
