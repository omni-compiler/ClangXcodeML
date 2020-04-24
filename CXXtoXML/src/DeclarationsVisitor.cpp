#include "XMLVisitorBase.h"
#include "NnsTableInfo.h"
#include "TypeTableInfo.h"
#include "DeclarationsVisitor.h"
#include "InheritanceInfo.h"
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
  setLocation(S->getBeginLoc());

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
      if (stmt) {
        TraverseStmt(stmt);
        xmlNewProp(xmlGetLastChild(curNode),
                   BAD_CAST "for_stmt_kind", BAD_CAST kind);
      }
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
    newProp("xcodemlType", optContext->typetableinfo.getTypeName(T).c_str());
  }

  if (auto CE = dyn_cast<clang::CastExpr>(S)) {
    newProp("clangCastKind", CE->getCastKindName());
  }

  if (const auto BE = dyn_cast<CXXBoolLiteralExpr>(S)) {
    newProp("bool_value", (BE->getValue() ? "true" : "false"));
  }

  if (const auto CDAE = dyn_cast<CXXDefaultArgExpr>(S)) {
    const auto E = CDAE->getExpr();
    TraverseStmt(E);
    return false;
  }

  if (const auto CDE = dyn_cast<CXXDeleteExpr>(S)) {
    newBoolProp("is_array_form", CDE->isArrayForm());
    newBoolProp("is_global_delete", CDE->isGlobalDelete());
  }

  if (auto OCE = dyn_cast<clang::CXXOperatorCallExpr>(S)) {
    newProp("xcodeml_operator_kind",
        OverloadedOperatorKindToString(OCE->getOperator(), OCE->getNumArgs()));
    if (OCE->getDirectCallee() == nullptr) {
      newBoolProp("is_member_function", false);
    }else{
      const auto is_member = isa<clang::CXXMethodDecl>(OCE->getDirectCallee());
      newBoolProp("is_member_function", is_member);
    }
  }

  if (auto NL = dyn_cast<CXXNewExpr>(S)) {
    newBoolProp("is_new_array", NL->isArray());
    newBoolProp("has_initializer", NL->hasInitializer());
    newBoolProp("is_global_new", NL->isGlobalNew());

  }

  if (auto ME = dyn_cast<clang::MemberExpr>(S)) {
    newBoolProp("is_arrow", ME->isArrow());

    const auto MD = ME->getMemberDecl();
    auto memberName = makeNameNode(optContext->typetableinfo, MD);
    xmlAddChild(curNode, memberName);

    if (const auto DRE = dyn_cast<clang::DeclRefExpr>(ME->getBase())) {
      const auto DN = DRE->getNameInfo().getName();
      newBoolProp("is_access_to_anon_record", DN.isEmpty());
    }
  }

  if (auto DRE = dyn_cast<DeclRefExpr>(S)) {
    const auto kind = DRE->getDecl()->getDeclKindName();
    newProp("declkind", kind);
    auto nameNode = makeNameNode(optContext->typetableinfo, DRE);

    const auto parent = DRE->getFoundDecl()->getDeclContext();
    assert(parent);
    xmlNewProp(nameNode,
        BAD_CAST "nns",
        BAD_CAST(optContext->nnstableinfo.getNnsName(parent).c_str()));

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
    const auto &CXT = mangleContext->getASTContext();
    const auto &SM = CXT.getSourceManager();
    const auto location = SM.getSpellingLoc(IL->getLocation());
    const auto spelling =
        clang::Lexer::getSpelling(location, buffer, SM, CXT.getLangOpts());
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

  if (auto SL = dyn_cast<clang::StringLiteral>(S)) {
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
    for (auto &range :  ILE->children()) {
      TraverseStmt(range);
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
        return true;
      } else {
        TraverseStmt(UEOTTE->getArgumentExpr());
        return false; // already traversed
      }
    }
    case UETT_PreferredAlignOf:
    case UETT_AlignOf: {
      newChild((UEOTTE->getKind()==UETT_PreferredAlignOf) ?
               "gccAlignOfExpr" : "AlignOfExpr");
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
    case UETT_OpenMPRequiredSimdAlign:
      //  NStmt("UnaryExprOrTypeTraitExpr(UETT_OpenMPRequiredSimdAlign");
      abort();
      return true;
    default:
      S->dump();
      abort();
    }
  }

  if (const auto LS = dyn_cast<LabelStmt>(S)) {
    newProp("label_name", LS->getName());
  }

  if (const auto GS = dyn_cast<GotoStmt>(S)) {
    const auto LD = GS->getLabel();
    assert(LD);
    const auto LS = LD->getStmt();
    assert(LS);
    newProp("label_name", LS->getName());
  }

  return true;
}

bool
DeclarationsVisitor::PreVisitType(QualType T) {
  if (T.isNull()) {
    newComment("Type:NULL");
    return true;
  }
  if (!xmlHasProp(curNode, BAD_CAST "type")) {
    newProp("type", optContext->typetableinfo.getTypeName(T).c_str());
  }
  return false;
}

bool
DeclarationsVisitor::PreVisitTypeLoc(TypeLoc TL) {
  newChild("clangTypeLoc");
  newProp("class", NameForTypeLoc(TL));
  const auto T = TL.getType();
  newProp("type", optContext->typetableinfo.getTypeName(T).c_str());
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
  if (D->isFirstDecl()) {
    newBoolProp("is_first_decl", true);
  }  if (D->isCanonicalDecl()) {
    newBoolProp("is_canonical_decl", true);
  }
  if (D->getAccess() != AS_none) {
    newProp("access", AccessSpec(D->getAccess()).c_str());
  } else {
    newBoolProp("clang_access_none", true);
  }

  if (isa<TranslationUnitDecl>(D)) {
    auto typetable = addChild("xcodemlTypeTable");
    optContext->typetableinfo.pushTypeTableStack(typetable);
    auto nnsTable = addChild("xcodemlNnsTable");
    optContext->nnstableinfo.pushNnsTableStack(nnsTable);
  }

  if (const auto LSD = dyn_cast<LinkageSpecDecl>(D)) {
    newProp("language_id", getLanguageIdAsString(LSD->getLanguage()));
  }

  NamedDecl *ND = dyn_cast<NamedDecl>(D);
  if (ND && !isa<UsingDirectiveDecl>(ND)) {
    auto nameNode = makeNameNode(optContext->typetableinfo, ND);

    /* experimental */
    const auto parent = ND->getDeclContext();
    assert(parent);
    xmlNewProp(nameNode,
        BAD_CAST "test_nns_decl_kind",
        BAD_CAST(parent->getDeclKindName()));
    xmlNewProp(nameNode,
        BAD_CAST "nns",
        BAD_CAST(optContext->nnstableinfo.getNnsName(parent).c_str()));

    xmlAddChild(curNode, nameNode);
  }

  if (auto UD = dyn_cast<UsingDecl>(D)) {
    newBoolProp("is_access_declaration", UD->isAccessDeclaration());
  }

  if (auto VD = dyn_cast<ValueDecl>(D)) {
    const auto T = VD->getType();
    newProp("xcodemlType", optContext->typetableinfo.getTypeName(T).c_str());
  }

  if (auto TD = dyn_cast<TypeDecl>(D)) {
    const auto T = QualType(TD->getTypeForDecl(), 0);
    newProp("xcodemlType", optContext->typetableinfo.getTypeName(T).c_str());
  }

  if (auto TND = dyn_cast<TypedefNameDecl>(D)) {
    const auto T = TND->getUnderlyingType();
    newProp("xcodemlTypedefType",
        optContext->typetableinfo.getTypeName(T).c_str());
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
    newBoolProp("has_external_storage", VD->hasExternalStorage());
    newBoolProp("has_init", VD->hasInit());
    newBoolProp("is_static_local", VD->isStaticLocal());
    newBoolProp("is_static_data_member", VD->isStaticDataMember());
    newBoolProp("is_out_of_line", VD->isOutOfLine());
  }

  if (auto FD = dyn_cast<FieldDecl>(D)) {
    newBoolProp("is_bit_field", FD->isBitField());
    newBoolProp("is_unnnamed_bit_field", FD->isUnnamedBitfield());
  }

  if (auto ND = dyn_cast<NamespaceDecl>(D)) {
    newBoolProp("is_inline", ND->isInline());
    newBoolProp("is_anonymous", ND->isAnonymousNamespace());
    if (!ND->isAnonymousNamespace()) {
      newBoolProp("is_first_declared", ND->isOriginalNamespace());
    }
  }
  if (const auto UDD = dyn_cast<UsingDirectiveDecl>(D)) {
    const auto ND = UDD->getNominatedNamespaceAsWritten();
    const auto nameNode = makeNameNode(optContext->typetableinfo, ND);
    xmlAddChild(curNode, nameNode);
  }
  if (auto FD = dyn_cast<FunctionDecl>(D)) {
    const auto ll = FD->getLanguageLinkage();
    if (ll != NoLanguageLinkage) {
      newProp("language_linkage", stringifyLanguageLinkage(ll));
    }
    newBoolProp("is_function_template_specialization",
        FD->isFunctionTemplateSpecialization());
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
      newProp(
          "parent_class", optContext->typetableinfo.getTypeName(T).c_str());
    } else {
      newBoolProp("clang_parent_class_not_found", true);
    }
  }

  if (isa<TemplateDecl>(D) || isa<ClassTemplatePartialSpecializationDecl>(D)) {
    auto typetable = addChild("xcodemlTypeTable");
    optContext->typetableinfo.pushTypeTableStack(typetable);
    auto nnsTable = addChild("xcodemlNnsTable");
    optContext->nnstableinfo.pushNnsTableStack(nnsTable);
  }
  return true;
}

bool
DeclarationsVisitor::PostVisitDecl(Decl *D) {
  if (!D) {
    return true;
  }
  if (isa<TemplateDecl>(D) || isa<ClassTemplatePartialSpecializationDecl>(D)
      || isa<TranslationUnitDecl>(D)) {
    optContext->typetableinfo.popTypeTableStack();
    optContext->nnstableinfo.popNnsTableStack();
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
    newProp(
        "clang_name_type", optContext->typetableinfo.getTypeName(T).c_str());
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

} // namespace

bool
DeclarationsVisitor::PreVisitNestedNameSpecifierLoc(NestedNameSpecifierLoc N) {
  const auto Spec = N.getNestedNameSpecifier();
  if (!Spec) {
    return true;
  }
  newChild("clangNestedNameSpecifier");
  const auto kind = SpecifierKindToString(Spec->getKind());
  newProp("clang_nested_name_specifier_kind", kind.c_str());

  switch (Spec->getKind()) {
  case NestedNameSpecifier::Identifier: {
    const auto Ident = Spec->getAsIdentifier();
    assert(Ident);
    newProp("token", Ident->getNameStart());
    break;
  }
  case NestedNameSpecifier::Namespace: {
    const auto ND = Spec->getAsNamespace();
    assert(ND);
    const auto nameNode = makeNameNode(optContext->typetableinfo, ND);
    xmlAddChild(curNode, nameNode);
    break;
  }
  case NestedNameSpecifier::TypeSpec: {
    const auto T = Spec->getAsType();
    assert(T);
    const auto dtident = optContext->typetableinfo.getTypeName(QualType(T, 0));
    newProp("xcodemlType", dtident.c_str());
    break;
  }
  default: break;
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
    newProp("xcodemlType", optContext->typetableinfo.getTypeName(T).c_str());
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
