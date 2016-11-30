#include "XMLVisitorBase.h"
#include "TypeTableVisitor.h"
#include "DeclarationsVisitor.h"
#include "InheritanceInfo.h"
#include "clang/Basic/Builtins.h"
#include "clang/Lex/Lexer.h"
#include <map>
#include <experimental/optional>
#include "OperationKinds.h"

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

  newChild("clangStmt");
  newProp("class", S->getStmtClassName());
  setLocation(S->getLocStart());

  const BinaryOperator *BO = dyn_cast<const BinaryOperator>(S);
  if (BO) {
    auto namePtr = BOtoElemName(BO->getOpcode());
    if (namePtr) {
      newProp("binOpName", namePtr->c_str());
    } else {
      auto opName = BinaryOperator::getOpcodeStr(BO->getOpcode());
      newProp("clangBinOpToken", opName.str().c_str());
    }
  }
  const UnaryOperator *UO = dyn_cast<const UnaryOperator>(S);
  if (UO) {
    auto namePtr = UOtoElemName(UO->getOpcode());
    if (namePtr) {
      newProp("unaryOpName", namePtr->c_str());
    } else {
      auto opName = UnaryOperator::getOpcodeStr(UO->getOpcode());
      newProp("clangUnaryOpToken", opName.str().c_str());
    }
  }

  if (isa<clang::Expr>(S)) {
    auto T = static_cast<Expr*>(S)->getType();
    newProp("xcodemlType", typetableinfo->getTypeName(T).c_str());
  }

  if (isa<IntegerLiteral>(S)) {
    const unsigned INIT_BUFFER_SIZE = 32;
    SmallVector<char, INIT_BUFFER_SIZE> buffer;
    auto loc = static_cast<IntegerLiteral*>(S)->getLocation();
    auto& CXT = mangleContext->getASTContext();
    auto spelling = clang::Lexer::getSpelling(
        loc,
        buffer,
        CXT.getSourceManager(),
        CXT.getLangOpts());
    newProp("token", spelling.str().c_str());
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
      newChild("clangStmt");
      newProp("class", "UnaryExprOrTypeTraitExpr_UETT_VecStep");
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
  newChild("clangDecl");
  newProp("class", D->getDeclKindName());
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

  newChild("clangDeclarationNameInfo",
          II ? II->getNameStart() : nullptr);
  newProp("class", NameForDeclarationName(DN));
  return true;
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
