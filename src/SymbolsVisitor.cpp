#include "XcodeMlVisitorBase.h"
#include "SymbolsVisitor.h"
#include "TypeTableVisitor.h"
#include "DeclarationsVisitor.h"

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

std::string NAttr(std::string mes);


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
  return true; // nothing to emit. traverse children recursively
}

bool
SymbolsVisitor::PreVisitType(QualType T) {
  (void)T;
  return false; // nothing to emit. do not traverse children
}

bool
SymbolsVisitor::PreVisitAttr(Attr *A) {
  if (!A) {
    newComment("Attr_NULL");
    return false;
  }
  newComment(NAttr(A->getSpelling()).c_str());
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
  if (D->isImplicit() && D->getKind() != Decl::Function) {
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
  case Decl::Label:
    {
      LabelDecl *LD = dyn_cast<LabelDecl>(D);
      newComment("Decl_Label");
      newChild("id");
      newProp("type", typetableinfo->getTypeNameForLabel().c_str());
      newProp("sclass", "label");
      if (LD) {
        IdentifierInfo *II = LD->getDeclName().getAsIdentifierInfo();
        if (II) {
          addChild("name", II->getNameStart());
        }
      }
      return false;
    }
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
  case Decl::Enum:
    {
      newComment("Decl_Enum");
      EnumDecl *ED = dyn_cast<EnumDecl>(D);
      if (!ED) {
        return false;
      }
      xmlNodePtr origCur = curNode;
      newChild("id");
      QualType T(ED->getTypeForDecl(), 0);
      newProp("type", typetableinfo->getTypeName(T).c_str());
      newProp("sclass", "tagname");
      if (ED) {
        IdentifierInfo *II = ED->getDeclName().getAsIdentifierInfo();
        if (II) {
          addChild("name", II->getNameStart());
        }
      }
      curNode = origCur;

      HookForDecl = [this, ED](Decl *D){

        if (D->getKind() == Decl::EnumConstant) {
          EnumConstantDecl *ECD = dyn_cast<EnumConstantDecl>(D);
          xmlNodePtr origCur = curNode;

          newComment("Decl_EnumConstant");
          newChild("id");
          QualType T(ED->getTypeForDecl(), 0);
          newProp("type", typetableinfo->getTypeName(T).c_str());
          newProp("sclass", "moe");
          if (ECD) {
            IdentifierInfo *II = ECD->getDeclName().getAsIdentifierInfo();
            if (II) {
              addChild("name", II->getNameStart());
            }
          }
          curNode = origCur;
          return true;
        } else {
          return TraverseDecl(D);
        }
      };
      return true; // traverse children
    }

  case Decl::Record:
    {
      RecordDecl *RD = dyn_cast<RecordDecl>(D);
      if (RD && !RD->isFirstDecl()) {
        newComment("Decl_Record (not 1st)");
        return false;
      }
      newComment("Decl_Record");
      newChild("id");
      if (RD) {
        QualType T(RD->getTypeForDecl(), 0);
        newProp("type", typetableinfo->getTypeName(T).c_str());
      }
      newProp("sclass", "tagname");
      if (RD) {
        IdentifierInfo *II = RD->getDeclName().getAsIdentifierInfo();
        if (II) {
          addChild("name", II->getNameStart());
        }
      }
      return false;
    }
  case Decl::CXXRecord: ND("Decl_CXXRecord");
  case Decl::ClassTemplateSpecialization: ND("Decl_ClassTemplateSpecialization");
  case Decl::ClassTemplatePartialSpecialization: ND("Decl_ClassTemplatePartialSpecialization");
  case Decl::TemplateTypeParm: ND("Decl_TemplateTypeParm");
  case Decl::TypeAlias: ND("Decl_TypeAlias");
  case Decl::Typedef:
    {
      TypedefDecl *TD = dyn_cast<TypedefDecl>(D);
      newComment("Decl_Typedef");
      if (TD && TD->getUnderlyingType()->isBuiltinType()) {
        return true; // do not emit this typedef
      }
      newChild("id");
      if (TD) {
        QualType T = TD->getUnderlyingType();
        newProp("type", typetableinfo->getTypeName(T).c_str());
      }
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
  case Decl::Field:
    // this is called from TypeTable (not from SymbolsVisitor's Decl::Record)
    {
      FieldDecl *FD = dyn_cast<FieldDecl>(D);
      newComment("Decl_Field");
      newChild("id");
      if (FD) {
        Expr *BW = nullptr;
        if (FD->isBitField()) {
          APSInt Value;
          ASTContext &Ctx = mangleContext->getASTContext();
          BW = FD->getBitWidth();
          if (dyn_cast<IntegerLiteral>(BW)
              && BW->EvaluateAsInt(Value, Ctx)) {
            newProp("bit_field", Value.toString(10).c_str());
            BW = nullptr;
          } else {
            newProp("bit_field", "*");
          }
        }
        QualType T = FD->getType();
        newProp("type", typetableinfo->getTypeName(T).c_str());
        if (FD->getParent()->getKind() == Decl::CXXRecord) {
          newProp("access", AccessSpec(D->getAccess()).c_str());
        }
        IdentifierInfo *II = FD->getDeclName().getAsIdentifierInfo();
        if (II) {
          addChild("name", II->getNameStart());
        }
        if (BW) {
          DeclarationsVisitor DV(mangleContext, curNode, "bitField", typetableinfo); 
          DV.TraverseStmt(BW);
        }
      }
      return true;
    }
  case Decl::ObjCAtDefsField: ND("Decl_ObjCAtDefsField");
  case Decl::ObjCIvar: ND("Decl_ObjCIvar");
  case Decl::Function:
    {
      FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
      if (!FD->isFirstDecl()) {
        newComment("Decl_Function: not 1st");
        IdentifierInfo *II = FD->getDeclName().getAsIdentifierInfo();
        if (II) {
          newComment(II->getNameStart());
        }
        return false;
      }
      newComment("Decl_Function");
      newChild("id");
      if (FD) {
        QualType T = FD->getType();
        newProp("type", typetableinfo->getTypeName(T).c_str());
      }
      newProp("sclass", "extern_def");
      if (FD) {
        IdentifierInfo *II = FD->getDeclName().getAsIdentifierInfo();
        if (II) {
          addChild("name", II->getNameStart());
        }
      }
      return false;
    }
  case Decl::CXXMethod:
  case Decl::CXXConstructor:
  case Decl::CXXConversion:
  case Decl::CXXDestructor:
    {
      CXXMethodDecl *MD = dyn_cast<CXXMethodDecl>(D);
      if (!MD->isFirstDecl()) {
        newComment("Decl_CXXMethod: not 1st");
        IdentifierInfo *II = MD->getDeclName().getAsIdentifierInfo();
        if (II) {
          newComment(II->getNameStart());
        }
        return false;
      }
      newComment("Decl_CXXMethod");
      newChild("id");
      if (MD) {
        QualType T = MD->getType();
        newProp("type", typetableinfo->getTypeName(T).c_str());
      }
      newProp("sclass", "extern_def");
      if (MD) {
        newProp("access", AccessSpec(MD->getAccess()).c_str());
        IdentifierInfo *II = MD->getDeclName().getAsIdentifierInfo();
        if (II) {
          addChild("name", II->getNameStart());
        }
      }

      return false;
    }
  case Decl::MSProperty: ND("Decl_MSProperty");
  case Decl::NonTypeTemplateParm: ND("Decl_NonTypeTemplateParm");
  case Decl::Var:
    {
      VarDecl *VD = dyn_cast<VarDecl>(D);
      const char *sclass;

      newComment("Decl_Var");
      newChild("id");
      if (VD) {
        QualType T = VD->getType();
        newProp("type", typetableinfo->getTypeName(T).c_str());
      }
      switch (VD->getStorageClass()) {
      case SC_None:
        switch (VD->getStorageDuration()) {
        case SD_FullExpression: sclass = "auto"; break;
        case SD_Automatic: sclass = "auto"; break;
        case SD_Thread: sclass = "extern_def"; break; //???
        case SD_Static: sclass = "extern_def"; break; // maybe OK
        case SD_Dynamic: sclass = "auto"; break; //???
        }
        break;
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
  case Decl::ParmVar:
    {
      ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(D);

      newComment("Decl_ParmVar");
      newChild("id");
      if (PVD) {
        QualType T = PVD->getType();
        newProp("type", typetableinfo->getTypeName(T).c_str());
      }
      newProp("sclass", "param");
      if (PVD) {
        IdentifierInfo *II = PVD->getDeclName().getAsIdentifierInfo();
        if (II) {
          addChild("name", II->getNameStart());
        }
      }
      return false;
      //return true;
    }
  case Decl::VarTemplateSpecialization: ND("Decl_VarTemplateSpecialization");
  case Decl::VarTemplatePartialSpecialization: ND("Decl_VarTemplatePartialSpecialization");
  case Decl::EnumConstant:
    // this is called from TypeTable (not from SymbolsVisitor's Decl::Enum)
    {
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

bool
SymbolsVisitor::PreVisitConstructorInitializer(CXXCtorInitializer *) {
  return true;
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
