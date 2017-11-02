#include "XMLVisitorBase.h"
#include "TypeTableInfo.h"
#include "DeclarationsVisitor.h"
#include "InheritanceInfo.h"
#include "NnsTableInfo.h"
#include "XcodeMlNameElem.h"
#include "clang/Basic/Builtins.h"
#include "clang/Lex/Lexer.h"
#include <map>
#include <sstream>
#include "ClangUtil.h"
#include "ClangOperator.h"

using namespace clang;
using namespace llvm;

static cl::opt<bool> OptTraceDeclarations("trace-declarations",
    cl::desc("emit traces on <globalDeclarations>, <declarations>"),
    cl::cat(CXX2XMLCategory));
static cl::opt<bool> OptDisableDeclarations("disable-declarations",
    cl::desc("disable <globalDeclarations>, <declarations>"),
    cl::cat(CXX2XMLCategory));

const char *
DeclarationsVisitor::getVisitorName() const {
  return OptTraceDeclarations ? "Declarations" : nullptr;
}

namespace {

std::string
getSpelling(clang::Expr *E, const clang::ASTContext &CXT) {
  const unsigned INIT_BUFFER_SIZE = 32;
  SmallVector<char, INIT_BUFFER_SIZE> buffer;
  auto spelling = clang::Lexer::getSpelling(
      E->getExprLoc(), buffer, CXT.getSourceManager(), CXT.getLangOpts());
  return spelling.str();
}

std::string
unsignedToHexString(unsigned u) {
  std::stringstream ss;
  ss << std::hex << "0x" << u;
  return ss.str();
}

} // namespace

bool
DeclarationsVisitor::PreVisitStmt(Stmt *S) {
  if (!S) {
    newComment("Stmt:NULL");
    return true;
  }

  newChild("clangStmt");
  newProp("class", S->getStmtClassName());
  setLocation(S->getLocStart());

  if (auto FS = dyn_cast<ForStmt>(S)) {
    const std::vector<std::tuple<const char *, Stmt *>> children = {
        std::make_tuple("init", FS->getInit()),
        std::make_tuple("cond", FS->getCond()),
        std::make_tuple("iter", FS->getInc()),
        std::make_tuple("body", FS->getBody()),
    };
    for (auto &child : children) {
      const char *kind;
      Stmt *stmt;
      std::tie(kind, stmt) = child;
      TraverseStmt(stmt);
      xmlNewProp(
          xmlGetLastChild(curNode), BAD_CAST "for_stmt_kind", BAD_CAST kind);
    }
    return false; // already traversed
  }

  const BinaryOperator *BO = dyn_cast<const BinaryOperator>(S);
  if (BO) {
    auto namePtr = BOtoElemName(BO->getOpcode());
    if (namePtr) {
      newProp("binOpName", namePtr);
    } else {
      auto opName = BinaryOperator::getOpcodeStr(BO->getOpcode());
      newProp("clangBinOpToken", opName.str().c_str());
    }
  }
  const UnaryOperator *UO = dyn_cast<const UnaryOperator>(S);
  if (UO) {
    auto namePtr = UOtoElemName(UO->getOpcode());
    if (namePtr) {
      newProp("unaryOpName", namePtr);
    } else {
      auto opName = UnaryOperator::getOpcodeStr(UO->getOpcode());
      newProp("clangUnaryOpToken", opName.str().c_str());
    }
  }

  if (auto E = dyn_cast<clang::Expr>(S)) {
    newProp("valueCategory",
        E->isXValue() ? "xvalue" : E->isRValue() ? "prvalue" : "lvalue");
    auto T = E->getType();
    newProp("xcodemlType", typetableinfo->getTypeName(T).c_str());
  }

  if (auto CE = dyn_cast<clang::CastExpr>(S)) {
    newProp("clangCastKind", CE->getCastKindName());
  }

  if (auto OCE = dyn_cast<clang::CXXOperatorCallExpr>(S)) {
    newProp("xcodeml_operator_kind",
        OverloadedOperatorKindToString(OCE->getOperator(), OCE->getNumArgs()));
  }

  if (auto NL = dyn_cast<CXXNewExpr>(S)) {
    newBoolProp("is_new_array", NL->isArray());
  }

  if (auto ME = dyn_cast<clang::MemberExpr>(S)) {
    newBoolProp("is_arrow", ME->isArrow());

    const auto MD = ME->getMemberDecl();
    auto memberName = makeNameNode(*typetableinfo, MD);
    xmlAddChild(curNode, memberName);

    if (const auto DRE = dyn_cast<clang::DeclRefExpr>(ME->getBase())) {
      const auto DN = DRE->getNameInfo().getName();
      newBoolProp("is_access_to_anon_record", DN.isEmpty());
    }
  }

  if (auto DRE = dyn_cast<DeclRefExpr>(S)) {
    const auto kind = DRE->getDecl()->getDeclKindName();
    newProp("declkind", kind);
    auto nameNode = makeNameNode(*typetableinfo, DRE);
    xmlAddChild(curNode, nameNode);
  }

  if (auto CL = dyn_cast<CharacterLiteral>(S)) {
    newProp(
        "hexadecimalNotation", unsignedToHexString(CL->getValue()).c_str());
    newProp("token", getSpelling(CL, mangleContext->getASTContext()).c_str());
  }

  if (auto IL = dyn_cast<IntegerLiteral>(S)) {
    const unsigned INIT_BUFFER_SIZE = 32;
    SmallVector<char, INIT_BUFFER_SIZE> buffer;
    auto &CXT = mangleContext->getASTContext();
    auto spelling = clang::Lexer::getSpelling(
        IL->getLocation(), buffer, CXT.getSourceManager(), CXT.getLangOpts());
    newProp("token", spelling.str().c_str());
    std::string decimalNotation = IL->getValue().toString(10, true);
    newProp("decimalNotation", decimalNotation.c_str());
  }

  if (auto FL = dyn_cast<FloatingLiteral>(S)) {
    const unsigned INIT_BUFFER_SIZE = 32;
    SmallVector<char, INIT_BUFFER_SIZE> buffer;
    auto &CXT = mangleContext->getASTContext();
    auto spelling = clang::Lexer::getSpelling(
        FL->getLocation(), buffer, CXT.getSourceManager(), CXT.getLangOpts());
    newProp("token", spelling.str().c_str());
  }

  if (auto SL = dyn_cast<StringLiteral>(S)) {
    StringRef Data = SL->getString();
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
    newProp("stringLiteral", literalAsString.c_str());
  }

  if (auto ILE = dyn_cast<InitListExpr>(S)) {
    /* `InitListExpr` has two kinds of children, `SyntacticForm`
     * and `SemanticForm`. Do not traverse `SyntacticForm`,
     * otherwise it emits the elements twice.
     */
    for (Stmt::child_range range = ILE->children(); range; ++range) {
      TraverseStmt(*range);
    }
    return false;
  }

  UnaryExprOrTypeTraitExpr *UEOTTE = dyn_cast<UnaryExprOrTypeTraitExpr>(S);
  if (UEOTTE) {
    // 7.8 sizeof, alignof
    switch (UEOTTE->getKind()) {
    case UETT_SizeOf: {
      newChild("sizeOfExpr");
      TraverseType(static_cast<Expr *>(S)->getType());
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
      TraverseType(static_cast<Expr *>(S)->getType());
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

      // case UETT_OpenMPRequiredSimdAlign:
      //  NStmt("UnaryExprOrTypeTraitExpr(UETT_OpenMPRequiredSimdAlign");
    }
  }

  return true;
}

bool
DeclarationsVisitor::PreVisitType(QualType T) {
  if (T.isNull()) {
    newComment("Type:NULL");
    return true;
  }
  newProp("type", typetableinfo->getTypeName(T).c_str());
  return false;
}

bool
DeclarationsVisitor::PreVisitTypeLoc(TypeLoc TL) {
  newChild("TypeLoc");
  newProp("class", NameForTypeLoc(TL));
  const auto T = TL.getType();
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

namespace {

const char *
getLanguageIdAsString(clang::LinkageSpecDecl::LanguageIDs id) {
  using clang::LinkageSpecDecl;
  switch (id) {
  case LinkageSpecDecl::lang_c: return "C";
  case LinkageSpecDecl::lang_cxx: return "C++";
  }
}

} // namespace

bool
DeclarationsVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return true;
  }

  // default: use the AST name simply.
  newChild("clangDecl");
  newProp("class", D->getDeclKindName());
  setLocation(D->getLocation());

  auto &CXT = mangleContext->getASTContext();
  auto &SM = CXT.getSourceManager();
  if (auto RC = CXT.getRawCommentForDeclNoCache(D)) {
    auto comment = static_cast<std::string>(RC->getRawText(SM));
    addChild("comment", comment.c_str());
  }

  if (D->isImplicit()) {
    newBoolProp("is_implicit", true);
  }
  if (D->getAccess() != AS_none) {
    newProp("access", AccessSpec(D->getAccess()).c_str());
  } else {
    newBoolProp("clang_access_none", true);
  }

  if (isa<TranslationUnitDecl>(D)) {
    auto typetable = addChild("xcodemlTypeTable");
    typetableinfo->pushTypeTableStack(typetable);
    auto nnsTable = addChild("xcodemlNnsTable");
    nnstableinfo->pushNnsTableStack(nnsTable);
  }

  if (const auto LSD = dyn_cast<LinkageSpecDecl>(D)) {
    newProp("language_id", getLanguageIdAsString(LSD->getLanguage()));
  }

  NamedDecl *ND = dyn_cast<NamedDecl>(D);
  if (ND) {
    auto nameNode = makeNameNode(*typetableinfo, ND);
    xmlAddChild(curNode, nameNode);
  }

  if (auto UD = dyn_cast<UsingDecl>(D)) {
    newBoolProp("is_access_declaration", UD->isAccessDeclaration());
  }

  if (auto VD = dyn_cast<ValueDecl>(D)) {
    const auto T = VD->getType();
    newProp("xcodemlType", typetableinfo->getTypeName(T).c_str());
  }

  if (auto TD = dyn_cast<TypeDecl>(D)) {
    const auto T = QualType(TD->getTypeForDecl(), 0);
    newProp("xcodemlType", typetableinfo->getTypeName(T).c_str());
  }

  if (auto TND = dyn_cast<TypedefNameDecl>(D)) {
    const auto T = TND->getUnderlyingType();
    newProp("xcodemlTypedefType", typetableinfo->getTypeName(T).c_str());
  }

  if (const auto RD = dyn_cast<RecordDecl>(D)) {
    newBoolProp("is_this_declaration_a_definition",
        RD->isThisDeclarationADefinition());
  }

  if (auto VD = dyn_cast<VarDecl>(D)) {
    const auto ll = VD->getLanguageLinkage();
    if (ll != NoLanguageLinkage) {
      newProp("language_linkage", stringifyLanguageLinkage(ll));
    }
    newBoolProp("has_init", VD->hasInit());
    newBoolProp("is_static_data_member", VD->isStaticDataMember());
    newBoolProp("is_out_of_line", VD->isOutOfLine());
  }

  if (auto ND = dyn_cast<NamespaceDecl>(D)) {
    newBoolProp("is_inline", ND->isInline());
    newBoolProp("is_anonymous", ND->isAnonymousNamespace());
    if (!ND->isAnonymousNamespace()) {
      newBoolProp("is_first_declared", ND->isOriginalNamespace());
    }
  }
  if (auto FD = dyn_cast<FunctionDecl>(D)) {
    const auto ll = FD->getLanguageLinkage();
    if (ll != NoLanguageLinkage) {
      newProp("language_linkage", stringifyLanguageLinkage(ll));
    }
    newBoolProp("is_defaulted", FD->isDefaulted());
    newBoolProp("is_deleted", FD->isDeletedAsWritten());
    newBoolProp("is_pure", FD->isPure());
    newBoolProp("is_variadic", FD->isVariadic());
  }
  if (auto MD = dyn_cast<CXXMethodDecl>(D)) {
    newBoolProp("is_const", MD->isConst());
    newBoolProp("is_static", MD->isStatic());
    newBoolProp("is_virtual", MD->isVirtual());
    if (auto RD = MD->getParent()) {
      assert(mangleContext);
      const auto T = mangleContext->getASTContext().getRecordType(RD);
      newProp("parent_class", typetableinfo->getTypeName(T).c_str());
    } else {
      newBoolProp("clang_parent_class_not_found", true);
    }
  }
  return true;
}

bool
DeclarationsVisitor::PostVisitDecl(Decl *D) {
  if (isa<TranslationUnitDecl>(D)) {
    typetableinfo->popTypeTableStack();
    nnstableinfo->popNnsTableStack();
  }
  return true;
}

bool
DeclarationsVisitor::PreVisitDeclarationNameInfo(DeclarationNameInfo NI) {
  DeclarationName DN = NI.getName();

  const auto name = NI.getAsString();
  newChild("clangDeclarationNameInfo", name.c_str());
  newProp("class", NameForDeclarationName(DN));
  newBoolProp("is_empty", DN.isEmpty());

  // FIXME: not MECE
  const auto T = DN.getCXXNameType();
  if (!T.isNull()) {
    newProp("clang_name_type", typetableinfo->getTypeName(T).c_str());
  }
  return true;
}

namespace {

std::string
SpecifierKindToString(clang::NestedNameSpecifier::SpecifierKind kind) {
  switch (kind) {
  case NestedNameSpecifier::Identifier: return "identifier";
  case NestedNameSpecifier::Namespace: return "namespace";
  case NestedNameSpecifier::NamespaceAlias: return "namespace_alias";
  case NestedNameSpecifier::TypeSpec: return "type_specifier";
  case NestedNameSpecifier::TypeSpecWithTemplate:
    return "type_specifier_with_template";
  case NestedNameSpecifier::Global: return "global";
  case NestedNameSpecifier::Super: return "MS_super";
  }
}

clang::IdentifierInfo *
getAsIdentifierInfo(clang::NestedNameSpecifier *NNS) {
  switch (NNS->getKind()) {
  case NestedNameSpecifier::Identifier: return NNS->getAsIdentifier();
  case NestedNameSpecifier::Namespace:
    return NNS->getAsNamespace()->getIdentifier();
  case NestedNameSpecifier::NamespaceAlias:
    return NNS->getAsNamespaceAlias()->getIdentifier();
  case NestedNameSpecifier::Super:
    return NNS->getAsRecordDecl()->getIdentifier();
  case NestedNameSpecifier::TypeSpec:
  case NestedNameSpecifier::TypeSpecWithTemplate:
  case NestedNameSpecifier::Global: return nullptr;
  }
}

} // namespace

bool
DeclarationsVisitor::PreVisitNestedNameSpecifierLoc(NestedNameSpecifierLoc N) {
  if (auto NNS = N.getNestedNameSpecifier()) {
    newChild("clangNestedNameSpecifier");
    newProp("kind", SpecifierKindToString(NNS->getKind()).c_str());
    newProp("nns", nnstableinfo->getNnsName(NNS).c_str());
    newProp("is_dependent", NNS->isDependent() ? "1" : "0");
    newProp("is_instantiation_dependent",
        NNS->isInstantiationDependent() ? "1" : "0");
    if (auto ident = getAsIdentifierInfo(NNS)) {
      newProp("name", ident->getNameStart());
    }
  }
  return true;
}

bool
DeclarationsVisitor::PreVisitConstructorInitializer(CXXCtorInitializer *CI) {
  if (!CI) {
    return true;
  }
  newChild("clangConstructorInitializer");
  newBoolProp("is_written", CI->isWritten());

  // FIXME: temporary implementation

  if (auto member = CI->getMember()) {
    newProp("member", member->getNameAsString().c_str());
  } else if (auto base = CI->getBaseClass()) {
    const auto T = QualType(base, 0);
    newProp("xcodemlType", typetableinfo->getTypeName(T).c_str());
  } else {
    newBoolProp("clang_unknown_ctor_init", true);
  }
  return true;
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
