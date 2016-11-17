#include "XMLVisitorBase.h"
#include "TypeTableVisitor.h"
#include "DeclarationsVisitor.h"
#include "InheritanceInfo.h"
#include "clang/Basic/Builtins.h"
#include <map>

using namespace clang;
using namespace llvm;

static cl::opt<bool>
OptTraceDeclarations("trace-declarations",
                     cl::desc("emit traces on <globalDeclarations>, <declarations>"),
                     cl::cat(CXX2XMLCategory));
static cl::opt<bool>
OptDisableDeclarations("disable-declarations",
                       cl::desc("disable <globalDeclarations>, <declarations>"),
                       cl::cat(CXX2XMLCategory));

const char *
DeclarationsVisitor::getVisitorName() const {
  return OptTraceDeclarations ? "Declarations" : nullptr;
}

// helper macros

#define NExpr(mes, content) do {                                        \
    newChild(mes, content);                                             \
    TraverseType(static_cast<Expr*>(S)->getType());                     \
    return true;                                                        \
  } while (0)
bool
DeclarationsVisitor::PreVisitStmt(Stmt *S) {
  if (!S) {
    newComment("Stmt:NULL");
    return true;
  }

  newChild((std::string("Stmt:") + S->getStmtClassName()).c_str());
  setLocation(S->getLocStart());

  const BinaryOperator *BO = dyn_cast<const BinaryOperator>(S);
  if (BO) {
    // XcodeML-C-0.9J.pdf: 7.6(assignExpr), 7.7, 7.10(commmaExpr)
    switch (BO->getOpcode()) {
    case BO_PtrMemD:   NExpr("memberPointerRef", nullptr);
    case BO_PtrMemI:   NExpr("memberIndirectRef", nullptr); // undefined by XcodeML
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
    switch (UO->getOpcode()) {
    case UO_PostInc:   NExpr("postIncrExpr", nullptr);
    case UO_PostDec:   NExpr("postDecrExpr", nullptr);
    case UO_PreInc:    NExpr("preIncrExpr", nullptr);
    case UO_PreDec:    NExpr("preDecrExpr", nullptr);
    case UO_AddrOf:    NExpr("AddrOfExpr", nullptr); // undefined by XcodeML
    case UO_Deref:     NExpr("pointerRef", nullptr);
    case UO_Plus:      NExpr("unaryPlusExpr", nullptr);
    case UO_Minus:     NExpr("unaryMinusExpr", nullptr);
    case UO_Not:       NExpr("bitNotExpr", nullptr);
    case UO_LNot:      NExpr("logNotExpr", nullptr);
    case UO_Real:      NExpr("unaryRealExpr", nullptr); // undefined by XcodeML
    case UO_Imag:      NExpr("unaryImagExpr", nullptr); // undefined by XcodeML
    case UO_Extension: NExpr("unrayExtensionExpr", nullptr); // undefined by XcodeML
    }
  }
  UnaryExprOrTypeTraitExpr *UEOTTE = dyn_cast<UnaryExprOrTypeTraitExpr>(S);
  if (UEOTTE) {
    //7.8 sizeof, alignof
    switch (UEOTTE->getKind()) {
    case UETT_SizeOf: {
      newChild("sizeOfExpr");
      TraverseType(static_cast<Expr*>(S)->getType());
      if (UEOTTE->isArgumentType()) {
        newChild("typeName");
        TraverseType(UEOTTE->getArgumentType());
      } else {
        TraverseStmt(UEOTTE->getArgumentExpr());
      }
      return true;
    }
    case UETT_AlignOf: {
      newChild("gccAlignOfExpr");
      TraverseType(static_cast<Expr*>(S)->getType());
      if (UEOTTE->isArgumentType()) {
        newChild("typeName");
        TraverseType(UEOTTE->getArgumentType());
      } else {
        TraverseStmt(UEOTTE->getArgumentExpr());
      }
      return true;

    }
    case UETT_VecStep:
      newChild("Stmt:UnaryExprOrTypeTraitExpr_UETT_VecStep");
      return true;

    //case UETT_OpenMPRequiredSimdAlign:
    //  NStmt("UnaryExprOrTypeTraitExpr(UETT_OpenMPRequiredSimdAlign");
    }
  }

  return true;
}
#undef NExpr

bool
DeclarationsVisitor::PreVisitType(QualType T) {
  if (T.isNull()) {
    newComment("Type:NULL");
    return true;
  }
  newProp("type", typetableinfo->getTypeName(T).c_str());
  return true;
}

bool
DeclarationsVisitor::PreVisitAttr(Attr *A) {
  if (!A) {
    newComment("Attr:NULL");
    return true;
  }
  newComment(std::string("Attr:") + A->getSpelling());
  newChild("gccAttribute");

  newProp("name", contentBySource(A->getLocation(), A->getLocation()).c_str());

  std::string prettyprint;
  raw_string_ostream OS(prettyprint);
  ASTContext &CXT = mangleContext->getASTContext();
  A->printPretty(OS, PrintingPolicy(CXT.getLangOpts()));
  newComment(OS.str());

  return true;
}

bool
DeclarationsVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return true;
  }

  // default: use the AST name simply.
  newChild((std::string("Decl:") + D->getDeclKindName()).c_str());    
  setLocation(D->getLocation());
  if (D->isImplicit()) {
    newProp("is_implicit", "1");
  }
  NamedDecl *ND = dyn_cast<NamedDecl>(D);
  if (ND) {
    addChild("fullName", ND->getQualifiedNameAsString().c_str());
  }
  return true;
}

bool
DeclarationsVisitor::PreVisitDeclarationNameInfo(DeclarationNameInfo NI) {
  DeclarationName DN = NI.getName();
  IdentifierInfo *II = DN.getAsIdentifierInfo();

  newChild((std::string("DeclarationNameInfo:")
            + NameForDeclarationName(DN)).c_str(),
          II ? II->getNameStart() : nullptr);
  return true;
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
