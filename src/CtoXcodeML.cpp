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

    const char *NameForStmt(Stmt *S) const {
        if (!S) {
            return "Stmt_NULL";
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
    const char *NameForType(QualType T) const {
        if (T.isNull()) {
            return "Type_NULL";
        }
        switch (T->getTypeClass()) {
        case Type::Builtin: return "Type_Builtin";
        case Type::Complex: return "Type_Complex";
        case Type::Pointer: return "Type_Pointer";
        case Type::BlockPointer: return "Type_BlockPointer";
        case Type::LValueReference: return "Type_LValueReference";
        case Type::RValueReference: return "Type_RValueReference";
        case Type::MemberPointer: return "Type_MemberPointer";
        case Type::ConstantArray: return "Type_ConstantArray";
        case Type::IncompleteArray: return "Type_IncompleteArray";
        case Type::VariableArray: return "Type_VariableArray";
        case Type::DependentSizedArray: return "Type_DependentSizedArray";
        case Type::DependentSizedExtVector: return "Type_DependentSizedExtVector";
        case Type::Vector: return "Type_Vector";
        case Type::ExtVector: return "Type_ExtVector";
        case Type::FunctionProto: return "Type_FunctionProto";
        case Type::FunctionNoProto: return "Type_FunctionNoProto";
        case Type::UnresolvedUsing: return "Type_UnresolvedUsing";
        case Type::Paren: return "Type_Paren";
        case Type::Typedef: return "Type_Typedef";
        case Type::Adjusted: return "Type_Adjusted";
        case Type::Decayed: return "Type_Decayed";
        case Type::TypeOfExpr: return "Type_TypeOfExpr";
        case Type::TypeOf: return "Type_TypeOf";
        case Type::Decltype: return "Type_Decltype";
        case Type::UnaryTransform: return "Type_UnaryTransform";
        case Type::Record: return "Type_Record";
        case Type::Enum: return "Type_Enum";
        case Type::Elaborated: return "Type_Elaborated";
        case Type::Attributed: return "Type_Attributed";
        case Type::TemplateTypeParm: return "Type_TemplateTypeParm";
        case Type::SubstTemplateTypeParm: return "Type_SubstTemplateTypeParm";
        case Type::SubstTemplateTypeParmPack: return "Type_SubstTemplateTypeParmPack";
        case Type::TemplateSpecialization: return "Type_TemplateSpecialization";
        case Type::Auto: return "Type_Auto";
        case Type::InjectedClassName: return "Type_InjectedClassName";
        case Type::DependentName: return "Type_DependentName";
        case Type::DependentTemplateSpecialization: return "Type_DependentTemplateSpecialization";
        case Type::PackExpansion: return "Type_PackExpansion";
        case Type::ObjCObject: return "Type_ObjCObject";
        case Type::ObjCInterface: return "Type_ObjCInterface";
        case Type::ObjCObjectPointer: return "Type_ObjCObjectPointer";
        case Type::Atomic: return "Type_Atomic";
        }
    }
    const char *NameForTypeLoc(TypeLoc TL) const {
        if (TL.isNull()) {
            return "TypeLoc_NULL";
        }
        switch (TL.getTypeLocClass()) {
        case TypeLoc::Qualified: return "TypeLoc_Qualified";
        case TypeLoc::Builtin: return "TypeLoc_Builtin";
        case TypeLoc::Complex: return "TypeLoc_Complex";
        case TypeLoc::Pointer: return "TypeLoc_Pointer";
        case TypeLoc::BlockPointer: return "TypeLoc_BlockPointer";
        case TypeLoc::LValueReference: return "TypeLoc_LValueReference";
        case TypeLoc::RValueReference: return "TypeLoc_RValueReference";
        case TypeLoc::MemberPointer: return "TypeLoc_MemberPointer";
        case TypeLoc::ConstantArray: return "TypeLoc_ConstantArray";
        case TypeLoc::IncompleteArray: return "TypeLoc_IncompleteArray";
        case TypeLoc::VariableArray: return "TypeLoc_VariableArray";
        case TypeLoc::DependentSizedArray: return "TypeLoc_DependentSizedArray";
        case TypeLoc::DependentSizedExtVector: return "TypeLoc_DependentSizedExtVector";
        case TypeLoc::Vector: return "TypeLoc_Vector";
        case TypeLoc::ExtVector: return "TypeLoc_ExtVector";
        case TypeLoc::FunctionProto: return "TypeLoc_FunctionProto";
        case TypeLoc::FunctionNoProto: return "TypeLoc_FunctionNoProto";
        case TypeLoc::UnresolvedUsing: return "TypeLoc_UnresolvedUsing";
        case TypeLoc::Paren: return "TypeLoc_Paren";
        case TypeLoc::Typedef: return "TypeLoc_Typedef";
        case TypeLoc::Adjusted: return "TypeLoc_Adjusted";
        case TypeLoc::Decayed: return "TypeLoc_Decayed";
        case TypeLoc::TypeOfExpr: return "TypeLoc_TypeOfExpr";
        case TypeLoc::TypeOf: return "TypeLoc_TypeOf";
        case TypeLoc::Decltype: return "TypeLoc_Decltype";
        case TypeLoc::UnaryTransform: return "TypeLoc_UnaryTransform";
        case TypeLoc::Record: return "TypeLoc_Record";
        case TypeLoc::Enum: return "TypeLoc_Enum";
        case TypeLoc::Elaborated: return "TypeLoc_Elaborated";
        case TypeLoc::Attributed: return "TypeLoc_Attributed";
        case TypeLoc::TemplateTypeParm: return "TypeLoc_TemplateTypeParm";
        case TypeLoc::SubstTemplateTypeParm: return "TypeLoc_SubstTemplateTypeParm";
        case TypeLoc::SubstTemplateTypeParmPack: return "TypeLoc_SubstTemplateTypeParmPack";
        case TypeLoc::TemplateSpecialization: return "TypeLoc_TemplateSpecialization";
        case TypeLoc::Auto: return "TypeLoc_Auto";
        case TypeLoc::InjectedClassName: return "TypeLoc_InjectedClassName";
        case TypeLoc::DependentName: return "TypeLoc_DependentName";
        case TypeLoc::DependentTemplateSpecialization: return "TypeLoc_DependentTemplateSpecialization";
        case TypeLoc::PackExpansion: return "TypeLoc_PackExpansion";
        case TypeLoc::ObjCObject: return "TypeLoc_ObjCObject";
        case TypeLoc::ObjCInterface: return "TypeLoc_ObjCInterface";
        case TypeLoc::ObjCObjectPointer: return "TypeLoc_ObjCObjectPointer";
        case TypeLoc::Atomic: return "TypeLoc_Atomic";
        }
    }
    const char *NameForAttr(Attr *A) const {
        if (!A) {
            return "Attr_NULL";
        }
        switch (A->getKind()) {
        case attr::NUM_ATTRS: return "";
        case attr::AMDGPUNumSGPR: return "Attr_AMDGPUNumSGPR";
        case attr::AMDGPUNumVGPR: return "Attr_AMDGPUNumVGPR";
        case attr::ARMInterrupt: return "Attr_ARMInterrupt";
        case attr::AcquireCapability: return "Attr_AcquireCapability";
        case attr::AcquiredAfter: return "Attr_AcquiredAfter";
        case attr::AcquiredBefore: return "Attr_AcquiredBefore";
        case attr::Alias: return "Attr_Alias";
        case attr::AlignMac68k: return "Attr_AlignMac68k";
        case attr::AlignValue: return "Attr_AlignValue";
        case attr::Aligned: return "Attr_Aligned";
        case attr::AlwaysInline: return "Attr_AlwaysInline";
        case attr::AnalyzerNoReturn: return "Attr_AnalyzerNoReturn";
        case attr::Annotate: return "Attr_Annotate";
        case attr::ArcWeakrefUnavailable: return "Attr_ArcWeakrefUnavailable";
        case attr::ArgumentWithTypeTag: return "Attr_ArgumentWithTypeTag";
        case attr::AsmLabel: return "Attr_AsmLabel";
        case attr::AssertCapability: return "Attr_AssertCapability";
        case attr::AssertExclusiveLock: return "Attr_AssertExclusiveLock";
        case attr::AssertSharedLock: return "Attr_AssertSharedLock";
        case attr::AssumeAligned: return "Attr_AssumeAligned";
        case attr::Availability: return "Attr_Availability";
        case attr::Blocks: return "Attr_Blocks";
        case attr::C11NoReturn: return "Attr_C11NoReturn";
        case attr::CDecl: return "Attr_CDecl";
        case attr::CFAuditedTransfer: return "Attr_CFAuditedTransfer";
        case attr::CFConsumed: return "Attr_CFConsumed";
        case attr::CFReturnsNotRetained: return "Attr_CFReturnsNotRetained";
        case attr::CFReturnsRetained: return "Attr_CFReturnsRetained";
        case attr::CFUnknownTransfer: return "Attr_CFUnknownTransfer";
        case attr::CUDAConstant: return "Attr_CUDAConstant";
        case attr::CUDADevice: return "Attr_CUDADevice";
        case attr::CUDAGlobal: return "Attr_CUDAGlobal";
        case attr::CUDAHost: return "Attr_CUDAHost";
        case attr::CUDAInvalidTarget: return "Attr_CUDAInvalidTarget";
        case attr::CUDALaunchBounds: return "Attr_CUDALaunchBounds";
        case attr::CUDAShared: return "Attr_CUDAShared";
        case attr::CXX11NoReturn: return "Attr_CXX11NoReturn";
        case attr::CallableWhen: return "Attr_CallableWhen";
        case attr::Capability: return "Attr_Capability";
        case attr::CapturedRecord: return "Attr_CapturedRecord";
        case attr::CarriesDependency: return "Attr_CarriesDependency";
        case attr::Cleanup: return "Attr_Cleanup";
        case attr::Cold: return "Attr_Cold";
        case attr::Common: return "Attr_Common";
        case attr::Const: return "Attr_Const";
        case attr::Constructor: return "Attr_Constructor";
        case attr::Consumable: return "Attr_Consumable";
        case attr::ConsumableAutoCast: return "Attr_ConsumableAutoCast";
        case attr::ConsumableSetOnRead: return "Attr_ConsumableSetOnRead";
        case attr::DLLExport: return "Attr_DLLExport";
        case attr::DLLImport: return "Attr_DLLImport";
        case attr::Deprecated: return "Attr_Deprecated";
        case attr::Destructor: return "Attr_Destructor";
        case attr::EnableIf: return "Attr_EnableIf";
        case attr::ExclusiveTrylockFunction: return "Attr_ExclusiveTrylockFunction";
        case attr::FallThrough: return "Attr_FallThrough";
        case attr::FastCall: return "Attr_FastCall";
        case attr::Final: return "Attr_Final";
        case attr::Flatten: return "Attr_Flatten";
        case attr::Format: return "Attr_Format";
        case attr::FormatArg: return "Attr_FormatArg";
        case attr::GNUInline: return "Attr_GNUInline";
        case attr::GuardedBy: return "Attr_GuardedBy";
        case attr::GuardedVar: return "Attr_GuardedVar";
        case attr::Hot: return "Attr_Hot";
        case attr::IBAction: return "Attr_IBAction";
        case attr::IBOutlet: return "Attr_IBOutlet";
        case attr::IBOutletCollection: return "Attr_IBOutletCollection";
        case attr::InitPriority: return "Attr_InitPriority";
        case attr::InitSeg: return "Attr_InitSeg";
        case attr::IntelOclBicc: return "Attr_IntelOclBicc";
        case attr::LockReturned: return "Attr_LockReturned";
        case attr::LocksExcluded: return "Attr_LocksExcluded";
        case attr::LoopHint: return "Attr_LoopHint";
        case attr::MSABI: return "Attr_MSABI";
        case attr::MSInheritance: return "Attr_MSInheritance";
        case attr::MSP430Interrupt: return "Attr_MSP430Interrupt";
        case attr::MSVtorDisp: return "Attr_MSVtorDisp";
        case attr::Malloc: return "Attr_Malloc";
        case attr::MaxFieldAlignment: return "Attr_MaxFieldAlignment";
        case attr::MayAlias: return "Attr_MayAlias";
        case attr::MinSize: return "Attr_MinSize";
        case attr::Mips16: return "Attr_Mips16";
        case attr::Mode: return "Attr_Mode";
        case attr::MsStruct: return "Attr_MsStruct";
        case attr::NSConsumed: return "Attr_NSConsumed";
        case attr::NSConsumesSelf: return "Attr_NSConsumesSelf";
        case attr::NSReturnsAutoreleased: return "Attr_NSReturnsAutoreleased";
        case attr::NSReturnsNotRetained: return "Attr_NSReturnsNotRetained";
        case attr::NSReturnsRetained: return "Attr_NSReturnsRetained";
        case attr::Naked: return "Attr_Naked";
        case attr::NoCommon: return "Attr_NoCommon";
        case attr::NoDebug: return "Attr_NoDebug";
        case attr::NoDuplicate: return "Attr_NoDuplicate";
        case attr::NoInline: return "Attr_NoInline";
        case attr::NoInstrumentFunction: return "Attr_NoInstrumentFunction";
        case attr::NoMips16: return "Attr_NoMips16";
        case attr::NoReturn: return "Attr_NoReturn";
        case attr::NoSanitizeAddress: return "Attr_NoSanitizeAddress";
        case attr::NoSanitizeMemory: return "Attr_NoSanitizeMemory";
        case attr::NoSanitizeThread: return "Attr_NoSanitizeThread";
        case attr::NoSplitStack: return "Attr_NoSplitStack";
        case attr::NoThreadSafetyAnalysis: return "Attr_NoThreadSafetyAnalysis";
        case attr::NoThrow: return "Attr_NoThrow";
        case attr::NonNull: return "Attr_NonNull";
        case attr::OMPThreadPrivateDecl: return "Attr_OMPThreadPrivateDecl";
        case attr::ObjCBridge: return "Attr_ObjCBridge";
        case attr::ObjCBridgeMutable: return "Attr_ObjCBridgeMutable";
        case attr::ObjCBridgeRelated: return "Attr_ObjCBridgeRelated";
        case attr::ObjCDesignatedInitializer: return "Attr_ObjCDesignatedInitializer";
        case attr::ObjCException: return "Attr_ObjCException";
        case attr::ObjCExplicitProtocolImpl: return "Attr_ObjCExplicitProtocolImpl";
        case attr::ObjCMethodFamily: return "Attr_ObjCMethodFamily";
        case attr::ObjCNSObject: return "Attr_ObjCNSObject";
        case attr::ObjCPreciseLifetime: return "Attr_ObjCPreciseLifetime";
        case attr::ObjCRequiresPropertyDefs: return "Attr_ObjCRequiresPropertyDefs";
        case attr::ObjCRequiresSuper: return "Attr_ObjCRequiresSuper";
        case attr::ObjCReturnsInnerPointer: return "Attr_ObjCReturnsInnerPointer";
        case attr::ObjCRootClass: return "Attr_ObjCRootClass";
        case attr::ObjCRuntimeName: return "Attr_ObjCRuntimeName";
        case attr::OpenCLImageAccess: return "Attr_OpenCLImageAccess";
        case attr::OpenCLKernel: return "Attr_OpenCLKernel";
        case attr::OptimizeNone: return "Attr_OptimizeNone";
        case attr::Overloadable: return "Attr_Overloadable";
        case attr::Override: return "Attr_Override";
        case attr::Ownership: return "Attr_Ownership";
        case attr::Packed: return "Attr_Packed";
        case attr::ParamTypestate: return "Attr_ParamTypestate";
        case attr::Pascal: return "Attr_Pascal";
        case attr::Pcs: return "Attr_Pcs";
        case attr::PnaclCall: return "Attr_PnaclCall";
        case attr::PtGuardedBy: return "Attr_PtGuardedBy";
        case attr::PtGuardedVar: return "Attr_PtGuardedVar";
        case attr::Pure: return "Attr_Pure";
        case attr::ReleaseCapability: return "Attr_ReleaseCapability";
        case attr::ReqdWorkGroupSize: return "Attr_ReqdWorkGroupSize";
        case attr::RequiresCapability: return "Attr_RequiresCapability";
        case attr::ReturnTypestate: return "Attr_ReturnTypestate";
        case attr::ReturnsNonNull: return "Attr_ReturnsNonNull";
        case attr::ReturnsTwice: return "Attr_ReturnsTwice";
        case attr::ScopedLockable: return "Attr_ScopedLockable";
        case attr::Section: return "Attr_Section";
        case attr::SelectAny: return "Attr_SelectAny";
        case attr::Sentinel: return "Attr_Sentinel";
        case attr::SetTypestate: return "Attr_SetTypestate";
        case attr::SharedTrylockFunction: return "Attr_SharedTrylockFunction";
        case attr::StdCall: return "Attr_StdCall";
        case attr::SysVABI: return "Attr_SysVABI";
        case attr::TLSModel: return "Attr_TLSModel";
        case attr::TestTypestate: return "Attr_TestTypestate";
        case attr::ThisCall: return "Attr_ThisCall";
        case attr::Thread: return "Attr_Thread";
        case attr::TransparentUnion: return "Attr_TransparentUnion";
        case attr::TryAcquireCapability: return "Attr_TryAcquireCapability";
        case attr::TypeTagForDatatype: return "Attr_TypeTagForDatatype";
        case attr::TypeVisibility: return "Attr_TypeVisibility";
        case attr::Unavailable: return "Attr_Unavailable";
        case attr::Unused: return "Attr_Unused";
        case attr::Used: return "Attr_Used";
        case attr::Uuid: return "Attr_Uuid";
        case attr::VecReturn: return "Attr_VecReturn";
        case attr::VecTypeHint: return "Attr_VecTypeHint";
        case attr::VectorCall: return "Attr_VectorCall";
        case attr::Visibility: return "Attr_Visibility";
        case attr::WarnUnused: return "Attr_WarnUnused";
        case attr::WarnUnusedResult: return "Attr_WarnUnusedResult";
        case attr::Weak: return "Attr_Weak";
        case attr::WeakImport: return "Attr_WeakImport";
        case attr::WeakRef: return "Attr_WeakRef";
        case attr::WorkGroupSizeHint: return "Attr_WorkGroupSizeHint";
        case attr::X86ForceAlignArgPointer: return "Attr_X86ForceAlignArgPointer";
        }
    }
    const char *NameForDecl(Decl *D) const {
        if (!D) {
            return "Decl_NULL";
        }
        switch (D->getKind()) {
        case Decl::AccessSpec: return "Decl_AccessSpec";
        case Decl::Block: return "Decl_Block";
        case Decl::Captured: return "Decl_Captured";
        case Decl::ClassScopeFunctionSpecialization: return "Decl_ClassScopeFunctionSpecialization";
        case Decl::Empty: return "Decl_Empty";
        case Decl::FileScopeAsm: return "Decl_FileScopeAsm";
        case Decl::Friend: return "Decl_Friend";
        case Decl::FriendTemplate: return "Decl_FriendTemplate";
        case Decl::Import: return "Decl_Import";
        case Decl::LinkageSpec: return "Decl_LinkageSpec";
        case Decl::Label: return "Decl_Label";
        case Decl::Namespace: return "Decl_Namespace";
        case Decl::NamespaceAlias: return "Decl_NamespaceAlias";
        case Decl::ObjCCompatibleAlias: return "Decl_ObjCCompatibleAlias";
        case Decl::ObjCCategory: return "Decl_ObjCCategory";
        case Decl::ObjCCategoryImpl: return "Decl_ObjCCategoryImpl";
        case Decl::ObjCImplementation: return "Decl_ObjCImplementation";
        case Decl::ObjCInterface: return "Decl_ObjCInterface";
        case Decl::ObjCProtocol: return "Decl_ObjCProtocol";
        case Decl::ObjCMethod: return "Decl_ObjCMethod";
        case Decl::ObjCProperty: return "Decl_ObjCProperty";
        case Decl::ClassTemplate: return "Decl_ClassTemplate";
        case Decl::FunctionTemplate: return "Decl_FunctionTemplate";
        case Decl::TypeAliasTemplate: return "Decl_TypeAliasTemplate";
        case Decl::VarTemplate: return "Decl_VarTemplate";
        case Decl::TemplateTemplateParm: return "Decl_TemplateTemplateParm";
        case Decl::Enum: return "Decl_Enum";
        case Decl::Record: return "Decl_Record";
        case Decl::CXXRecord: return "Decl_CXXRecord";
        case Decl::ClassTemplateSpecialization: return "Decl_ClassTemplateSpecialization";
        case Decl::ClassTemplatePartialSpecialization: return "Decl_ClassTemplatePartialSpecialization";
        case Decl::TemplateTypeParm: return "Decl_TemplateTypeParm";
        case Decl::TypeAlias: return "Decl_TypeAlias";
        case Decl::Typedef: return "Decl_Typedef";
        case Decl::UnresolvedUsingTypename: return "Decl_UnresolvedUsingTypename";
        case Decl::Using: return "Decl_Using";
        case Decl::UsingDirective: return "Decl_UsingDirective";
        case Decl::UsingShadow: return "Decl_UsingShadow";
        case Decl::Field: return "Decl_Field";
        case Decl::ObjCAtDefsField: return "Decl_ObjCAtDefsField";
        case Decl::ObjCIvar: return "Decl_ObjCIvar";
        case Decl::Function: return "Decl_Function";
        case Decl::CXXMethod: return "Decl_CXXMethod";
        case Decl::CXXConstructor: return "Decl_CXXConstructor";
        case Decl::CXXConversion: return "Decl_CXXConversion";
        case Decl::CXXDestructor: return "Decl_CXXDestructor";
        case Decl::MSProperty: return "Decl_MSProperty";
        case Decl::NonTypeTemplateParm: return "Decl_NonTypeTemplateParm";
        case Decl::Var: return "Decl_Var";
        case Decl::ImplicitParam: return "Decl_ImplicitParam";
        case Decl::ParmVar: return "Decl_ParmVar";
        case Decl::VarTemplateSpecialization: return "Decl_VarTemplateSpecialization";
        case Decl::VarTemplatePartialSpecialization: return "Decl_VarTemplatePartialSpecialization";
        case Decl::EnumConstant: return "Decl_EnumConstant";
        case Decl::IndirectField: return "Decl_IndirectField";
        case Decl::UnresolvedUsingValue: return "Decl_UnresolvedUsingValue";
        case Decl::OMPThreadPrivate: return "Decl_OMPThreadPrivate";
        case Decl::ObjCPropertyImpl: return "Decl_ObjCPropertyImpl";
        case Decl::StaticAssert: return "Decl_StaticAssert";
        case Decl::TranslationUnit: return nullptr; // no need to create a node
        }
    }
    const char *NameForNestedNameSpecifier(NestedNameSpecifier *NNS) const {
        if (!NNS) {
            return "NestedNameSpecifier_NULL";
        }
        switch (NNS->getKind()) {
        case NestedNameSpecifier::Identifier: return "NestedNameSpecifier_Identifier";
        case NestedNameSpecifier::Namespace: return "NestedNameSpecifier_Namespace";
        case NestedNameSpecifier::NamespaceAlias: return "NestedNameSpecifier_NamespaceAlias";
        case NestedNameSpecifier::Global: return "NestedNameSpecifier_Global";
        case NestedNameSpecifier::Super: return "NestedNameSpecifier_Super";
        case NestedNameSpecifier::TypeSpec: return "NestedNameSpecifier_TypeSpec";
        case NestedNameSpecifier::TypeSpecWithTemplate: return "NestedNameSpecifier_TypeSpecWithTemplate";
        }
    }
    const char *NameForNestedNameSpecifierLoc(NestedNameSpecifierLoc NNS) const {
        if (!NNS) {
            return "NestedNameSpecifierLOC_NULL";
        }
        switch (NNS.getNestedNameSpecifier()->getKind()) {
        case NestedNameSpecifier::Identifier: return "NestedNameSpecifierLoc_Identifier";
        case NestedNameSpecifier::Namespace: return "NestedNameSpecifierLoc_Namespace";
        case NestedNameSpecifier::NamespaceAlias: return "NestedNameSpecifierLoc_NamespaceAlias";
        case NestedNameSpecifier::Global: return "NestedNameSpecifierLoc_Global";
        case NestedNameSpecifier::Super: return "NestedNameSpecifierLoc_Super";
        case NestedNameSpecifier::TypeSpec: return "NestedNameSpecifierLoc_TypeSpec";
        case NestedNameSpecifier::TypeSpecWithTemplate: return "NestedNameSpecifierLoc_TypeSpecWithTemplate";
        }
    }
    const char *NameForDeclarationNameInfo(DeclarationNameInfo NameInfo) const {
        switch (NameInfo.getName().getNameKind()) {
        case DeclarationName::CXXConstructorName: return "DeclarationName_CXXConstructorName";
        case DeclarationName::CXXDestructorName: return "DeclarationName_CXXDestructorName";
        case DeclarationName::CXXConversionFunctionName: return "DeclarationName_CXXConversionFunctionName";
        case DeclarationName::Identifier: return "DeclarationName_Identifier";
        case DeclarationName::ObjCZeroArgSelector: return "DeclarationName_ObjCZeroArgSelector";
        case DeclarationName::ObjCOneArgSelector: return "DeclarationName_ObjCOneArgSelector";
        case DeclarationName::ObjCMultiArgSelector: return "DeclarationName_ObjCMultiArgSelector";
        case DeclarationName::CXXOperatorName: return "DeclarationName_CXXOperatorName";
        case DeclarationName::CXXLiteralOperatorName: return "DeclarationName_CXXLiteralOperatorName";
        case DeclarationName::CXXUsingDirective: return "DeclarationName_CXXUsingDirective";
            break;
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
