#include "XcodeMlVisitorBase.h"
#include "SymbolsVisitor.h"

using namespace clang;
using namespace llvm;

static cl::opt<bool>
OptTraceSymbols("trace-symbols",
                cl::desc("emit traces on <globalSymbols>, <symbols>"),
                cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableSymbols("disable-symbols",
                  cl::desc("disable <globalSymbols>, <symbols>"),
                  cl::cat(C2XcodeMLCategory));

#define ND(mes) do {newComment(mes); return false;} while (0)
#define NDeclName(mes) do {                                     \
    newComment(mes);                                            \
    newChild("name", nameString);                               \
    return true;                                                \
  } while (0)

#define NAttr(mes) newComment("Attr_" mes); break

const char *
SymbolsVisitor::getVisitorName() const {
  return OptTraceSymbols ? "Symbols" : nullptr;
}

bool
SymbolsVisitor::PreVisitStmt(Stmt *S) {
  // traverse Decls only
  return S && S->getStmtClass() == Stmt::DeclStmtClass;
}

bool
SymbolsVisitor::PreVisitTypeLoc(TypeLoc TL) {
  (void)TL;
  return true; // do not traverse children
}

bool
SymbolsVisitor::PreVisitAttr(Attr *A) {
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

  std::string attrName = contentBySource(A->getLocation(), A->getLocation());
  newProp("name", attrName.c_str());

  std::string prettyprint;
  raw_string_ostream OS(prettyprint);
  ASTContext &CXT = mangleContext->getASTContext();
  A->printPretty(OS, PrintingPolicy(CXT.getLangOpts()));
  newComment(OS.str().c_str());

  return true;
}

bool
SymbolsVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return false;
  }
  HookForAttr = [this](Attr *A){
      newChild("gccAttributes");
      HookForAttr = nullptr;
      return TraverseAttr(A);
  };

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
  case Decl::Record: {
    RecordDecl *RD = dyn_cast<RecordDecl>(D);
    newComment("Decl_Record");
    newChild("id");
    newProp("sclass", "tagname");
    if (RD) {
      IdentifierInfo *II = RD->getDeclName().getAsIdentifierInfo();
      if (II) {
        addChild("name", II->getNameStart());
      }
    }
    return true;
  }
  case Decl::CXXRecord: ND("Decl_CXXRecord");
  case Decl::ClassTemplateSpecialization: ND("Decl_ClassTemplateSpecialization");
  case Decl::ClassTemplatePartialSpecialization: ND("Decl_ClassTemplatePartialSpecialization");
  case Decl::TemplateTypeParm: ND("Decl_TemplateTypeParm");
  case Decl::TypeAlias: ND("Decl_TypeAlias");
  case Decl::Typedef: {
    TypedefDecl *TD = dyn_cast<TypedefDecl>(D);
    newComment("Decl_Typedef");
    newChild("id");
    newProp("sclass", "typedef_name");
    if (TD) {
      IdentifierInfo *II = TD->getDeclName().getAsIdentifierInfo();
      if (II) {
        addChild("name", II->getNameStart());
      }
    }
    return true;
  }
  case Decl::UnresolvedUsingTypename: ND("Decl_UnresolvedUsingTypename");
  case Decl::Using: ND("Decl_Using");
  case Decl::UsingDirective: ND("Decl_UsingDirective");
  case Decl::UsingShadow: ND("Decl_UsingShadow");
  case Decl::Field: {
    FieldDecl *FD = dyn_cast<FieldDecl>(D);
    newComment("Decl_Field");
    newChild("id");
    if (FD) {
      IdentifierInfo *II = FD->getDeclName().getAsIdentifierInfo();
      if (II) {
        addChild("name", II->getNameStart());
      }
    }
    return true;
  }
  case Decl::ObjCAtDefsField: ND("Decl_ObjCAtDefsField");
  case Decl::ObjCIvar: ND("Decl_ObjCIvar");
  case Decl::Function: {
    FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
    newComment("Decl_Function");
    newChild("id");
    newProp("sclass", "extern_def");
    if (FD) {
      IdentifierInfo *II = FD->getDeclName().getAsIdentifierInfo();
      if (II) {
        addChild("name", II->getNameStart());
      }
    }
    return true;
  }
  case Decl::CXXMethod: ND("Decl_CXXMethod");
  case Decl::CXXConstructor: ND("Decl_CXXConstructor");
  case Decl::CXXConversion: ND("Decl_CXXConversion");
  case Decl::CXXDestructor: ND("Decl_CXXDestructor");
  case Decl::MSProperty: ND("Decl_MSProperty");
  case Decl::NonTypeTemplateParm: ND("Decl_NonTypeTemplateParm");
  case Decl::Var: {
    VarDecl *VD = dyn_cast<VarDecl>(D);
    const char *sclass;

    newComment("Decl_Var");
    newChild("id");
    switch (VD->getStorageClass()) {
    case SC_None: sclass = "auto"; break;
    case SC_Extern: sclass = "extern_def"; break; // "extern"??
    case SC_Static: sclass = "static"; break;
    case SC_PrivateExtern: sclass = "extern"; break; //??
    case SC_OpenCLWorkGroupLocal: sclass = "SC_OpenCLWorkGroupLocal"; break;
    case SC_Auto: sclass = "auto"; break;
    case SC_Register: sclass = "register"; break;
    }
    newProp("sclass", sclass);
    if (VD) {
      IdentifierInfo *II = VD->getDeclName().getAsIdentifierInfo();
      if (II) {
        addChild("name", II->getNameStart());
      }
    }
    return true;
  }
  case Decl::ImplicitParam: ND("Decl_ImplicitParam");
  case Decl::ParmVar: {
    ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(D);

    newComment("Decl_ParmVar");
    newChild("id");
    newProp("sclass", "param");
    if (PVD) {
      IdentifierInfo *II = PVD->getDeclName().getAsIdentifierInfo();
      if (II) {
        addChild("name", II->getNameStart());
      }
    }
    return true;
  }
  case Decl::VarTemplateSpecialization: ND("Decl_VarTemplateSpecialization");
  case Decl::VarTemplatePartialSpecialization: ND("Decl_VarTemplatePartialSpecialization");
  case Decl::EnumConstant: {
    EnumConstantDecl *ECD = dyn_cast<EnumConstantDecl>(D);

    newComment("Decl_EnumConstant");
    newChild("id");
    if (ECD) {
      IdentifierInfo *II = ECD->getDeclName().getAsIdentifierInfo();
      if (II) {
        addChild("name", II->getNameStart());
      }
    }
    return true;
  }
  case Decl::IndirectField: ND("Decl_IndirectField");
  case Decl::UnresolvedUsingValue: ND("Decl_UnresolvedUsingValue");
  case Decl::OMPThreadPrivate: ND("Decl_OMPThreadPrivate");
  case Decl::ObjCPropertyImpl: ND("Decl_ObjCPropertyImpl");
  case Decl::StaticAssert: ND("Decl_StaticAssert");
  case Decl::TranslationUnit:
    if (OptDisableSymbols) {
      return false; // stop traverse
    } else {
      return true; // no need to create a child
    }
  }
}

bool
SymbolsVisitor::PreVisitNestedNameSpecifierLoc(NestedNameSpecifierLoc NNSL) {
  (void)NNSL;
  return true;
}

bool
SymbolsVisitor::PreVisitDeclarationNameInfo(DeclarationNameInfo NameInfo) {
#if 0
  DeclarationName DN = NameInfo.getName();
  IdentifierInfo *II = DN.getAsIdentifierInfo();
  const char *nameString = nullptr;
  if (II) {
    nameString = II->getNameStart().c_str();
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
#else
  (void)NameInfo;
  return true; // do nothing
#endif
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
