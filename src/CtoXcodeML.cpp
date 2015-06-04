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

    bool PostVisitStmt(const Stmt *S) {
        // do not add me as a child of the root node
        return true;
    }
    bool PostVisitDecl(const Decl *D) {
        // do not add me as a child of the root node
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

    const char *NameForUnaryOperator(const UnaryOperator *UnOp) const {
        // XcodeML-C-0.9J.pdf 7.2(varAddr), 7.3(pointerRef), 7.8, 7.11
        const char *Nam;

        switch (UnOp->getOpcode()) {
        case UO_PostInc:   Nam = "postIncrExpr"; break;
        case UO_PostDec:   Nam = "postDecrExpr"; break;
        case UO_PreInc:    Nam = "preIncrExpr"; break;
        case UO_PreDec:    Nam = "preDecrExpr"; break;
        case UO_AddrOf:    Nam = "varAddr"; break;
        case UO_Deref:     Nam = "pointerRef"; break;
        case UO_Plus:      Nam = "UNDEF_UO_Plus"; break;
        case UO_Minus:     Nam = "unaryMinusExpr"; break;
        case UO_Not:       Nam = "bitNotExpr"; break;
        case UO_LNot:      Nam = "logNotExpr"; break;
        case UO_Real:      Nam = "UNDEF_UO_Real"; break;
        case UO_Imag:      Nam = "UNDEF_UO_Imag"; break;
        case UO_Extension: Nam = "UNDEF_UO_Extension"; break;
        }
        //newComment((std::string("VisitUnaryOperator ")
        //           + std::string(Nam)).c_str());
        //setName(Nam);
        //setLocation(UnOp);
        //return true;
        return Nam;
    }

    const char *NameForBinaryOperator(const BinaryOperator *BinOp) const {
        // XcodeML-C-0.9J.pdf: 7.6(assignExpr), 7.7, 7.10(commmaExpr)
        const char *Nam;

        switch (BinOp->getOpcode()) {
        case BO_PtrMemD:   Nam = "UNDEF_BO_PtrMemD"; break;
        case BO_PtrMemI:   Nam = "UNDEF_BO_PtrMemI"; break;
        case BO_Mul:       Nam = "mulExpr"; break;
        case BO_Div:       Nam = "divExpr"; break;
        case BO_Rem:       Nam = "modExpr"; break;
        case BO_Add:       Nam = "plusExpr"; break;
        case BO_Sub:       Nam = "minusExpr"; break;
        case BO_Shl:       Nam = "LshiftExpr"; break;
        case BO_Shr:       Nam = "RshiftExpr"; break;
        case BO_LT:        Nam = "logLTExpr"; break;
        case BO_GT:        Nam = "logGTExpr"; break;
        case BO_LE:        Nam = "logLEExpr"; break;
        case BO_GE:        Nam = "logGEExpr"; break;
        case BO_EQ:        Nam = "logEQExpr"; break;
        case BO_NE:        Nam = "logNEQExpr"; break;
        case BO_And:       Nam = "bitAndExpr"; break;
        case BO_Xor:       Nam = "bitXorExpr"; break;
        case BO_Or:        Nam = "bitOrExpr"; break;
        case BO_LAnd:      Nam = "logAndExpr"; break;
        case BO_LOr:       Nam = "logOrExpr"; break;
        case BO_Assign:    Nam = "assignExpr"; break;
        case BO_Comma:     Nam = "commaExpr"; break;
        case BO_MulAssign: Nam = "asgMulExpr"; break;
        case BO_DivAssign: Nam = "asgDivExpr"; break;
        case BO_RemAssign: Nam = "asgModExpr"; break;
        case BO_AddAssign: Nam = "asgPlusExpr"; break;
        case BO_SubAssign: Nam = "asgMinusExpr"; break;
        case BO_ShlAssign: Nam = "asgLshiftExpr"; break;
        case BO_ShrAssign: Nam = "asgRshiftExpr"; break;
        case BO_AndAssign: Nam = "asgBitAndExpr"; break;
        case BO_OrAssign:  Nam = "asgBitOrExpr"; break;
        case BO_XorAssign: Nam = "asgBitXorExpr"; break;
        }
        //newComment((std::string("VisitBinaryOperator ")
        //           + std::string(Nam)).c_str());
        //setName(Nam);
        //setLocation(BinOp);
        //return true;
        return Nam;
    }

    const char *
    NameForBinaryConditionalOperator(const BinaryConditionalOperator *ConOp)
        const {
        //newComment("VisitBinaryConditionalOperator");
        //setName("UNDEF_BinaryConditionalOperator");
        //setLocation(ConOp);
        //return true;
        return "UNDEF_BinaryConditionalOperator";
    }
    bool
    PostVisitBinaryConditionalOperator(const BinaryConditionalOperator *ConOp)
    {
        newComment("PostVisitBinaryConditionalOperator");
        newComment(curNode->name);
        // x ?: y  ->  desugar to x ? x : y
        xmlNodePtr lhs = curNode->xmlChildrenNode;
        while (lhs && lhs->type == XML_COMMENT_NODE) {
            lhs = lhs->next;
        }
        if (lhs) {
            newComment("PVBCO: desugar: x ?: y -> x ? x : y");
            xmlNodePtr mid = xmlCopyNode(lhs, 1);
            if (mid) {
                newComment("PVBCO: copied node", mid);
                xmlAddNextSibling(lhs, mid);
            }
        }
        xmlAddChild(rootNode, curNode);
        return true;
    }
    const char *NameForConditionalOperator(const ConditionalOperator *ConOp)
        const {
        //newComment("VisitConditionalOperator");

        // XcodeML-C-0.9J.pdf 7.13
        //setName("condExpr");
        //setLocation(ConOp);
        //return true;
        return "condExpr";
    }
    const char *NameForCompoundStmt(CompoundStmt *S) {
        return "compoundStatement";
    }
    bool PostVisitParenExpr(ParenExpr *E) {
        errs() << "HOGEHOGEHOGEHOGEHOGEEEEEEE!!!!!!\n";
        xmlAddChild(rootNode, curNode);
        return true;
    }

#if 0
    bool PostVisitCompoundStmt(CompoundStmt *S) {
        newComment("VisitCompoundStmt");

        // XcodeML-C-0.9J.pdf 6.2
        XcodeMlSymbolsVisitor SV(this->astContext, this->curNode, "symbols");
        if (!OptDisableXSV) {
            SV.XcodeMlTraverseCompoundStmt(S);
        }
        //XcodeMlDeclarationsVisitor DV(this->astContext, this->curNode,
        //                              "Declarations");
        //DV.XcodeMlTraverseCompoundStmt(S);
        //setName("compoundStatement");
        return true;
    }
    bool VisitExpr(const Expr *E) {
        newComment("VisitExpr");
        //setName("zExpr");
        //setLocation(E);
        return true;
    }
#endif
    const char *NameForStmt(const Stmt *S) const {
        const BinaryOperator *BO = dyn_cast<const BinaryOperator>(S);
        if (BO) {
            return NameForBinaryOperator(BO);
        }
        const UnaryOperator *UO = dyn_cast<const UnaryOperator>(S);
        if (UO) {
            return NameForUnaryOperator(UO);
        }
        switch (S->getStmtClass()) {
        case Stmt::NoStmtClass:     return "Stmt_NoStmtClass";
        case Stmt::GCCAsmStmtClass: return "Stmt_GCCAsmStmtClass";
        case Stmt::MSAsmStmtClass:  return "Stmt_MSAsmStmtClass";
        case Stmt::AttributedStmtClass: return "Stmt_AttributedStmtClass";
        case Stmt::BreakStmtClass: return "Stmt_BreakStmtClass";
        case Stmt::CXXCatchStmtClass: return "";
        case Stmt::CXXForRangeStmtClass: return "";
        case Stmt::CXXTryStmtClass: return "";
        case Stmt::CapturedStmtClass: return "";
        case Stmt::CompoundStmtClass: return "";
        case Stmt::ContinueStmtClass: return "";
        case Stmt::DeclStmtClass: return "";
        case Stmt::DoStmtClass: return "";
        case Stmt::BinaryConditionalOperatorClass: return "";
        case Stmt::ConditionalOperatorClass: return "";
        case Stmt::AddrLabelExprClass: return "";
        case Stmt::ArraySubscriptExprClass: return "";
        case Stmt::ArrayTypeTraitExprClass: return "";
        case Stmt::AsTypeExprClass: return "";
        case Stmt::AtomicExprClass: return "";
        case Stmt::BinaryOperatorClass: return "";
        case Stmt::CompoundAssignOperatorClass: return "";
        case Stmt::BlockExprClass: return "";
        case Stmt::CXXBindTemporaryExprClass: return "";
        case Stmt::CXXBoolLiteralExprClass: return "";
        case Stmt::CXXConstructExprClass: return "";
        case Stmt::CXXTemporaryObjectExprClass: return "";
        case Stmt::CXXDefaultArgExprClass: return "";
        case Stmt::CXXDefaultInitExprClass: return "";
        case Stmt::CXXDeleteExprClass: return "";
        case Stmt::CXXDependentScopeMemberExprClass: return "";
        case Stmt::CXXNewExprClass: return "";
        case Stmt::CXXNoexceptExprClass: return "";
        case Stmt::CXXNullPtrLiteralExprClass: return "";
        case Stmt::CXXPseudoDestructorExprClass: return "";
        case Stmt::CXXScalarValueInitExprClass: return "";
        case Stmt::CXXStdInitializerListExprClass: return "";
        case Stmt::CXXThisExprClass: return "";
        case Stmt::CXXThrowExprClass: return "";
        case Stmt::CXXTypeidExprClass: return "";
        case Stmt::CXXUnresolvedConstructExprClass: return "";
        case Stmt::CXXUuidofExprClass: return "";
        case Stmt::CallExprClass: return "";
        case Stmt::CUDAKernelCallExprClass: return "";
        case Stmt::CXXMemberCallExprClass: return "";
        case Stmt::CXXOperatorCallExprClass: return "";
        case Stmt::UserDefinedLiteralClass: return "";
        case Stmt::CStyleCastExprClass: return "";
        case Stmt::CXXFunctionalCastExprClass: return "";
        case Stmt::CXXConstCastExprClass: return "";
        case Stmt::CXXDynamicCastExprClass: return "";
        case Stmt::CXXReinterpretCastExprClass: return "";
        case Stmt::CXXStaticCastExprClass: return "";
        case Stmt::ObjCBridgedCastExprClass: return "";
        case Stmt::ImplicitCastExprClass: return "";
        case Stmt::CharacterLiteralClass: return "";
        case Stmt::ChooseExprClass: return "";
        case Stmt::CompoundLiteralExprClass: return "";
        case Stmt::ConvertVectorExprClass: return "";
        case Stmt::DeclRefExprClass: return "";
        case Stmt::DependentScopeDeclRefExprClass: return "";
        case Stmt::DesignatedInitExprClass: return "";
        case Stmt::ExprWithCleanupsClass: return "";
        case Stmt::ExpressionTraitExprClass: return "";
        case Stmt::ExtVectorElementExprClass: return "";
        case Stmt::FloatingLiteralClass: return "";
        case Stmt::FunctionParmPackExprClass: return "";
        case Stmt::GNUNullExprClass: return "";
        case Stmt::GenericSelectionExprClass: return "";
        case Stmt::ImaginaryLiteralClass: return "";
        case Stmt::ImplicitValueInitExprClass: return "";
        case Stmt::InitListExprClass: return "";
        case Stmt::IntegerLiteralClass: return "";
        case Stmt::LambdaExprClass: return "";
        case Stmt::MSPropertyRefExprClass: return "";
        case Stmt::MaterializeTemporaryExprClass: return "";
        case Stmt::MemberExprClass: return "";
        case Stmt::ObjCArrayLiteralClass: return "";
        case Stmt::ObjCBoolLiteralExprClass: return "";
        case Stmt::ObjCBoxedExprClass: return "";
        case Stmt::ObjCDictionaryLiteralClass: return "";
        case Stmt::ObjCEncodeExprClass: return "";
        case Stmt::ObjCIndirectCopyRestoreExprClass: return "";
        case Stmt::ObjCIsaExprClass: return "";
        case Stmt::ObjCIvarRefExprClass: return "";
        case Stmt::ObjCMessageExprClass: return "";
        case Stmt::ObjCPropertyRefExprClass: return "";
        case Stmt::ObjCProtocolExprClass: return "";
        case Stmt::ObjCSelectorExprClass: return "";
        case Stmt::ObjCStringLiteralClass: return "";
        case Stmt::ObjCSubscriptRefExprClass: return "";
        case Stmt::OffsetOfExprClass: return "";
        case Stmt::OpaqueValueExprClass: return "";
        case Stmt::UnresolvedLookupExprClass: return "";
        case Stmt::UnresolvedMemberExprClass: return "";
        case Stmt::PackExpansionExprClass: return "";
        case Stmt::ParenExprClass: return "";
        case Stmt::ParenListExprClass: return "";
        case Stmt::PredefinedExprClass: return "";
        case Stmt::PseudoObjectExprClass: return "";
        case Stmt::ShuffleVectorExprClass: return "";
        case Stmt::SizeOfPackExprClass: return "";
        case Stmt::StmtExprClass: return "";
        case Stmt::StringLiteralClass: return "";
        case Stmt::SubstNonTypeTemplateParmExprClass: return "";
        case Stmt::SubstNonTypeTemplateParmPackExprClass: return "";
        case Stmt::TypeTraitExprClass: return "";
        case Stmt::UnaryExprOrTypeTraitExprClass: return "";
        case Stmt::UnaryOperatorClass: return "";
        case Stmt::VAArgExprClass: return "";
        case Stmt::ForStmtClass: return "";
        case Stmt::GotoStmtClass: return "";
        case Stmt::IfStmtClass: return "";
        case Stmt::IndirectGotoStmtClass: return "";
        case Stmt::LabelStmtClass: return "";
        case Stmt::MSDependentExistsStmtClass: return "";
        case Stmt::NullStmtClass: return "";
        case Stmt::OMPBarrierDirectiveClass: return "";
        case Stmt::OMPCriticalDirectiveClass: return "";
        case Stmt::OMPFlushDirectiveClass: return "";
        case Stmt::OMPForDirectiveClass: return "";
        case Stmt::OMPMasterDirectiveClass: return "";
        case Stmt::OMPParallelDirectiveClass: return "";
        case Stmt::OMPParallelForDirectiveClass: return "";
        case Stmt::OMPParallelSectionsDirectiveClass: return "";
        case Stmt::OMPSectionDirectiveClass: return "";
        case Stmt::OMPSectionsDirectiveClass: return "";
        case Stmt::OMPSimdDirectiveClass: return "";
        case Stmt::OMPSingleDirectiveClass: return "";
        case Stmt::OMPTaskDirectiveClass: return "";
        case Stmt::OMPTaskwaitDirectiveClass: return "";
        case Stmt::OMPTaskyieldDirectiveClass: return "";
        case Stmt::ObjCAtCatchStmtClass: return "";
        case Stmt::ObjCAtFinallyStmtClass: return "";
        case Stmt::ObjCAtSynchronizedStmtClass: return "";
        case Stmt::ObjCAtThrowStmtClass: return "";
        case Stmt::ObjCAtTryStmtClass: return "";
        case Stmt::ObjCAutoreleasePoolStmtClass: return "";
        case Stmt::ObjCForCollectionStmtClass: return "";
        case Stmt::ReturnStmtClass: return "";
        case Stmt::SEHExceptStmtClass: return "";
        case Stmt::SEHFinallyStmtClass: return "";
        case Stmt::SEHLeaveStmtClass: return "";
        case Stmt::SEHTryStmtClass: return "";
        case Stmt::CaseStmtClass: return "";
        case Stmt::DefaultStmtClass: return "";
        case Stmt::SwitchStmtClass: return "";
        case Stmt::WhileStmtClass: return "";
        }
    }

    bool VisitType(const Type *T) {
        newComment("VisitType");
        //setName("zType");
        return true;
    }

    bool VisitDecl(const Decl *D) {
        const char *Nam = D->getDeclKindName();
        newComment((std::string("VisitDecl ") + std::string(Nam)).c_str());
        //setName((std::string("Decl") + std::string(Nam)).c_str());
        setLocation(D);
        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *D) {
        // XcodeML-C-0.9J.pdf 5.1
        newComment("VisitFunctionDecl");
        setName("functionDefinition");
        DeclarationNameInfo DNI = D->getNameInfo();
        //setLocation(DNI.getLoc());

        const char *FuncName = DNI.getName().getAsString().c_str();
        xmlNewChild(curNode, nullptr, BAD_CAST "name", BAD_CAST FuncName);

        XcodeMlSymbolsVisitor SV(this->astContext, this->curNode, "symbols");
        if (!OptDisableXSV) {
            SV.BridgeDecl(D);
        }

        // should be handle <param> properly

        xmlAddChild(rootNode, curNode);
        curNode = xmlNewChild(curNode, nullptr, BAD_CAST "body", nullptr);
        return true;
    }
    bool PostVisitFunctionDecl(FunctionDecl *D) {
        // do not add me as a child of the root node
        return true;
    }
};


class XcodeMlASTConsumer : public ASTConsumer {
    const xmlNodePtr rootNode;

    xmlNodePtr addNewChild(const char *Name) {
        return xmlNewChild(rootNode, nullptr, BAD_CAST Name, nullptr);
    }

public:
    explicit XcodeMlASTConsumer(xmlNodePtr N) : rootNode(N) {};

    virtual void HandleTranslationUnit(ASTContext &CXT) override {
        XcodeMlTypeTableVisitor TTV(CXT, addNewChild("TypeTable"));
        XcodeMlSymbolsVisitor SV(CXT, addNewChild("globalSymbols"));
        XcodeMlDeclarationsVisitor DV(CXT, addNewChild("globalDeclarations"));

        Decl *D = CXT.getTranslationUnitDecl();

        if (!OptDisableXTTV) {
            TTV.BridgeDecl(D);
        }
        if (!OptDisableXSV) {
            SV.BridgeDecl(D);
        }
        if (!OptDisableXDV) {
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

    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI,
                                           StringRef file) override {
        return new XcodeMlASTConsumer(xmlDocGetRootElement(xmlDoc));
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
    Tool.setArgumentsAdjuster(new clang::tooling::ClangSyntaxOnlyAdjuster());

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
