#include "clang/AST/AST.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Driver/Options.h"
#include "clang/Lex/Lexer.h"
#include <sstream>
#include <string>
#include <type_traits>
#include <unistd.h>
#include <iostream>

#include "CXXtoXML.h"
#include "XMLRecursiveASTVisitor.h"

using namespace clang;
using namespace llvm;

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
XMLRecursiveASTVisitor::VisitStmt(Stmt *S) {
  if (!S) {
    newComment("Stmt:NULL");
    return true;
  }

#if 0
  if(debug_flag){
      fprintf(stderr,"-- VisitStmt(0x%p) --:\n",(void *)S); S->dump();
  }
#endif
  newChild("clangStmt");
  newProp("class", S->getStmtClassName());
  setLocation(S->getBeginLoc());

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
    newProp("xcodemlType", typetableinfo.getTypeName(T).c_str());
  }

  if (auto CE = dyn_cast<clang::CastExpr>(S)) {
    newProp("clangCastKind", CE->getCastKindName());
  }

  if (const auto BE = dyn_cast<CXXBoolLiteralExpr>(S)) {
    newProp("bool_value", (BE->getValue() ? "true" : "false"));
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
    auto memberName = makeNameNode(typetableinfo, MD);
    xmlAddChild(curNode, memberName);
    if (const auto DRE = dyn_cast<clang::DeclRefExpr>(ME->getBase())) {
      const auto DN = DRE->getNameInfo().getName();
      newBoolProp("is_access_to_anon_record", DN.isEmpty());
    }
  }

  if (auto DRE = dyn_cast<DeclRefExpr>(S)) {
    const auto kind = DRE->getDecl()->getDeclKindName();
    const auto RefedDecl = DRE->getDecl();
    //DRE->getDecl()->dump();
    newProp("declkind", kind);
    auto nameNode = makeNameNode(typetableinfo, DRE);
    const auto parent = DRE->getFoundDecl()->getDeclContext();
    assert(parent);
    xmlNewProp(nameNode,
        BAD_CAST "nns",
        BAD_CAST(nnstableinfo.getNnsName(parent).c_str()));

    xmlAddChild(curNode, nameNode);
  }
  if (auto LE = dyn_cast<LambdaExpr>(S)){
      for(const auto & cap: LE->captures()){
          auto kind = cap.getCaptureKind();
          std::string name;
          std::string flag;
          auto capnode = xmlNewNode(nullptr, BAD_CAST "Capture");
          xmlAddChild(curNode, capnode);
      }
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
XMLRecursiveASTVisitor::VisitType(QualType T) {
  if (T.isNull()) {
    newComment("Type:NULL");
    return true;
  }
  if (!xmlHasProp(curNode, BAD_CAST "type")) {
    newProp("type", typetableinfo.getTypeName(T).c_str());
  }

  

  return true;
}

bool
XMLRecursiveASTVisitor::VisitTypeLoc(TypeLoc TL) {
  newChild("clangTypeLoc");
  newProp("class", NameForTypeLoc(TL));
  const auto T = TL.getType();
  newProp("type", typetableinfo.getTypeName(T).c_str());
  return true;
}

bool
XMLRecursiveASTVisitor::VisitAttr(Attr *A) {
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
  return "NULL";
}

} // namespace

bool
XMLRecursiveASTVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return true;
  }

#if 0
  // llvm::errs() << "-- VisitDecl ---:\n"; D->dump();
  fprintf(stderr,"-- VisitDecl(0x%p) --:\n",(void *)D); D->dump();
#endif

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
    typetableinfo.pushTypeTableStack(typetable);
    auto nnsTable = addChild("xcodemlNnsTable");
    nnstableinfo.pushNnsTableStack(nnsTable);
  }

  if (const auto LSD = dyn_cast<LinkageSpecDecl>(D)) {
    newProp("language_id", getLanguageIdAsString(LSD->getLanguage()));
  }

  NamedDecl *ND = dyn_cast<NamedDecl>(D);
  if (ND && !isa<UsingDirectiveDecl>(ND)) {
    auto nameNode = makeNameNode(typetableinfo, ND);

    /* experimental */
    const auto parent = ND->getDeclContext();
    assert(parent);
    xmlNewProp(nameNode,
        BAD_CAST "test_nns_decl_kind",
        BAD_CAST(parent->getDeclKindName()));
    xmlNewProp(nameNode,
        BAD_CAST "nns",
        BAD_CAST(nnstableinfo.getNnsName(parent).c_str()));

    xmlAddChild(curNode, nameNode);
  }

  if (auto UD = dyn_cast<UsingDecl>(D)) {
    newBoolProp("is_access_declaration", UD->isAccessDeclaration());
  }

  if (auto VD = dyn_cast<ValueDecl>(D)) {
    const auto T = VD->getType();
    newProp("xcodemlType", typetableinfo.getTypeName(T).c_str());
  }

  if (auto TD = dyn_cast<TypeDecl>(D)) {
    const auto T = QualType(TD->getTypeForDecl(), 0);
    newProp("xcodemlType", typetableinfo.getTypeName(T).c_str());
  }

  if (auto TND = dyn_cast<TypedefNameDecl>(D)) {
    const auto T = TND->getUnderlyingType();
    newProp("xcodemlTypedefType",
	    typetableinfo.getTypeName(T).c_str());
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
    newBoolProp("is_constexpr", VD->isConstexpr());
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
    const auto nameNode = makeNameNode(typetableinfo, ND);
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
          "parent_class", typetableinfo.getTypeName(T).c_str());
    } else {
      newBoolProp("clang_parent_class_not_found", true);
    }
  }

  if (isa<TemplateDecl>(D) || isa<ClassTemplatePartialSpecializationDecl>(D)) {
    auto typetable = addChild("xcodemlTypeTable");
    typetableinfo.pushTypeTableStack(typetable);
    auto nnsTable = addChild("xcodemlNnsTable");
    nnstableinfo.pushNnsTableStack(nnsTable);
  }

  return true;
}

bool
XMLRecursiveASTVisitor::PostVisitDecl(Decl *D) {
  if (!D) {
    return true;
  }

  if (isa<TemplateDecl>(D) || isa<ClassTemplatePartialSpecializationDecl>(D)
      || isa<TranslationUnitDecl>(D)) {
    typetableinfo.popTypeTableStack();
    nnstableinfo.popNnsTableStack();
  }
  return true;
}
bool
XMLRecursiveASTVisitor::VisitDeclarationNameInfo(DeclarationNameInfo NI) {
  DeclarationName DN = NI.getName();

  const auto name = NI.getAsString();
  newChild("clangDeclarationNameInfo", name.c_str());
  newProp("class", NameForDeclarationName(DN));
  newBoolProp("is_empty", DN.isEmpty());

  // FIXME: not MECE
  const auto T = DN.getCXXNameType();
  if (!T.isNull()) {
    newProp(
        "clang_name_type", typetableinfo.getTypeName(T).c_str());
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
  return "NULL";
}

} // namespace

bool
XMLRecursiveASTVisitor::TraverseNestedNameSpecifierLoc(NestedNameSpecifierLoc N) {
  if(!N)
    return true;
  auto save = curNode;

  newChild("clangNestedNameSpecifier");
  if(NestedNameSpecifierLoc Prefix = N.getPrefix())
    TraverseNestedNameSpecifierLoc(Prefix);

  const auto Spec = N.getNestedNameSpecifier();
  if (!Spec) {
    return true;
  }

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
    const auto nameNode = makeNameNode(typetableinfo, ND);
    xmlAddChild(curNode, nameNode);
    break;
  }
  case NestedNameSpecifier::TypeSpecWithTemplate:
  case NestedNameSpecifier::TypeSpec: {
    const auto T = Spec->getAsType();
    assert(T);
    const auto dtident = typetableinfo.getTypeName(QualType(T, 0));
    newProp("xcodemlType", dtident.c_str());
    TraverseTypeLoc(N.getTypeLoc());
    break;
  }
  default: break;
  }
  curNode = save;
  return true;
}

bool
XMLRecursiveASTVisitor::VisitConstructorInitializer(CXXCtorInitializer *CI) {
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
    newProp("xcodemlType", typetableinfo.getTypeName(T).c_str());
  } else {
    newBoolProp("clang_unknown_ctor_init", true);
  }
  return true;
}

//
// Functions for XML
//
xmlNodePtr
XMLRecursiveASTVisitor::addChild(const char *Name, const char *Content) {
  return xmlNewTextChild(curNode, nullptr, BAD_CAST Name, BAD_CAST Content);
}

void
XMLRecursiveASTVisitor::newChild(const char *Name, const char *Content) {
    // printf("newChild for %s: %s on %p -> ", Name, Content, (void *)curNode);
    curNode = xmlNewTextChild(curNode, nullptr, BAD_CAST Name, BAD_CAST Content);
    // printf("%p\n",(void *)curNode);
}

void
XMLRecursiveASTVisitor::newProp(const char *Name, int Val, xmlNodePtr N) {
  if (!N)
    N = curNode;
  const auto buf = std::to_string(Val);
  xmlNewProp(N, BAD_CAST Name, BAD_CAST(buf.c_str()));
}

void
XMLRecursiveASTVisitor::newProp(const char *Name, const char *Val, xmlNodePtr N) {
  if (!N)
    N = curNode;
  xmlNewProp(N, BAD_CAST Name, BAD_CAST Val);
}

void
XMLRecursiveASTVisitor::newBoolProp(const char *Name, bool Val, xmlNodePtr N) {
  if (Val)
    newProp(Name, "1", N);
}

void
XMLRecursiveASTVisitor::newComment(const xmlChar *str, xmlNodePtr N) {
  if (!N)
    N = curNode;
#if 0
  if (const auto pName = getVisitorName()) {
    std::stringstream ss;
    ss << pName << "::" << str;
    xmlNodePtr Comment = xmlNewComment(BAD_CAST(ss.str().c_str()));
    xmlAddChild(N, Comment);
    // errs() << (const char *)Buf << "\n";
  }
#endif
}

void
XMLRecursiveASTVisitor::newComment(const char *str, xmlNodePtr N) {
  newComment(BAD_CAST str, N);
}

void
XMLRecursiveASTVisitor::newComment(const std::string &str, xmlNodePtr N) {
  newComment(str.c_str(), N);
}

void
XMLRecursiveASTVisitor::setLocation(SourceLocation Loc, xmlNodePtr N) {
  if (!N)
    N = curNode;
  FullSourceLoc FLoc = mangleContext->getASTContext().getFullLoc(Loc);
  if (FLoc.isValid()) {
    PresumedLoc PLoc = FLoc.getManager().getPresumedLoc(FLoc);

    newProp("column", PLoc.getColumn(), N);
    newProp("lineno", PLoc.getLine(), N);
    {
      const char *filename = PLoc.getFilename();
      static char cwd[BUFSIZ];
      static size_t cwdlen;

      if (cwdlen == 0) {
        getcwd(cwd, sizeof(cwd));
        cwdlen = strlen(cwd);
      }
      if (strncmp(filename, cwd, cwdlen) == 0 && filename[cwdlen] == '/') {
        newProp("file", filename + cwdlen + 1, N);
      } else {
        newProp("file", filename, N);
      }
    }
  }
}

std::string
XMLRecursiveASTVisitor::contentBySource(
    SourceLocation LocStart, SourceLocation LocEnd) {
  ASTContext &CXT = mangleContext->getASTContext();
  SourceManager &SM = CXT.getSourceManager();
  SourceLocation LocEndOfToken =
      Lexer::getLocForEndOfToken(LocEnd, 0, SM, CXT.getLangOpts());
  if (LocEndOfToken.isValid()) {
    const char *b = SM.getCharacterData(LocStart);
    const char *e = SM.getCharacterData(LocEndOfToken);
    if (e > b && e < b + 256) {
      return std::string(b, e - b);
    } else {
      return std::string("");
    }
  } else {
    return std::string("");
  }
}

//
// NameForXXX
//
const char *
XMLRecursiveASTVisitor::NameForStmt(clang::Stmt *S) {
    return S ? S->getStmtClassName() : "NULL";
}

const char *
XMLRecursiveASTVisitor::NameForType(clang::QualType QT) {
  return !QT.isNull() ? QT->getTypeClassName() : "NULL";
}


const char *
XMLRecursiveASTVisitor::NameForTypeLoc(clang::TypeLoc TL) {
  return !TL.isNull() ? TL.getType()->getTypeClassName() : "NULL";
}


const char *
XMLRecursiveASTVisitor::NameForAttr(clang::Attr *A) {
  return A ? A->getSpelling() : "NULL";
}

const char *
XMLRecursiveASTVisitor::NameForDecl(clang::Decl *D) {
  return D ? D->getDeclKindName() : "NULL";
}

const char *
XMLRecursiveASTVisitor::NameForNestedNameSpecifier(clang::NestedNameSpecifier *NS) {
  if (!NS)
    return "NULL";
  switch (NS->getKind()) {
  case clang::NestedNameSpecifier::Identifier: return "Identifier";
  case clang::NestedNameSpecifier::Namespace: return "Namespace";
  case clang::NestedNameSpecifier::NamespaceAlias: return "NamespaceAlias";
  case clang::NestedNameSpecifier::Global: return "Global";
  case clang::NestedNameSpecifier::Super: return "Super";
  case clang::NestedNameSpecifier::TypeSpec: return "TypeSpec";
  case clang::NestedNameSpecifier::TypeSpecWithTemplate:
    return "TypeSpecWithTemplate";
  }
  return "NULL";
}

const char *
XMLRecursiveASTVisitor::NameForNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NL) {
  return NameForNestedNameSpecifier(NL.getNestedNameSpecifier());
}

const char *
XMLRecursiveASTVisitor::NameForDeclarationName(clang::DeclarationName DN) {
  switch (DN.getNameKind()) {
  case clang::DeclarationName::CXXConstructorName:
    return "CXXConstructorName";
  case clang::DeclarationName::CXXDestructorName: return "CXXDestructorName";
  case clang::DeclarationName::CXXConversionFunctionName:
    return "CXXConversionFunctionName";
  case clang::DeclarationName::Identifier: return "Identifier";
  case clang::DeclarationName::ObjCZeroArgSelector:
    return "ObjCZeroArgSelector";
  case clang::DeclarationName::ObjCOneArgSelector:
    return "ObjCOneArgSelector";
  case clang::DeclarationName::ObjCMultiArgSelector:
    return "ObjCMultiArgSelector";
  case clang::DeclarationName::CXXOperatorName: return "CXXOperatorName";
  case clang::DeclarationName::CXXLiteralOperatorName:
    return "CXXLiteralOperatorName";
  case clang::DeclarationName::CXXUsingDirective: return "CXXUsingDirective";
  }
  return "NULL";
}

const char *
XMLRecursiveASTVisitor::NameForDeclarationNameInfo(clang::DeclarationNameInfo DN) {
  return NameForDeclarationName(DN.getName());
}

const char *
XMLRecursiveASTVisitor::NameForTemplateName(clang::TemplateName TN) {
  (void)TN;
  return "X";
}

const char *
XMLRecursiveASTVisitor::NameForTemplateArgument(const clang::TemplateArgument &TA) {
  (void)TA;
  return "X";
}

const char *
XMLRecursiveASTVisitor::NameForTemplateArgumentLoc(const clang::TemplateArgumentLoc &TL) {
  (void)TL;
  return "X";
}

const char *
XMLRecursiveASTVisitor::NameForConstructorInitializer(clang::CXXCtorInitializer *CI) {
  (void)CI;
  return "X";
}

bool
XMLRecursiveASTVisitor::SourceLocForStmt(clang::Stmt *S, clang::SourceLocation &SL) {
  if (S) {
    SL = S->getBeginLoc();
    return true;
  } else {
    return false;
  }
}

bool
XMLRecursiveASTVisitor::SourceLocForType(clang::QualType QT, clang::SourceLocation &SL) {
  (void)QT, (void)SL;
  return false;
}

bool
XMLRecursiveASTVisitor::SourceLocForTypeLoc(clang::TypeLoc TL, clang::SourceLocation &SL) {
  SL = TL.getBeginLoc();
  return true;
}

bool
XMLRecursiveASTVisitor::SourceLocForAttr(clang::Attr *A, clang::SourceLocation &SL) {
  if (A) {
    SL = A->getLocation();
    return true;
  } else {
    return false;
  }
}

bool
XMLRecursiveASTVisitor::SourceLocForDecl(clang::Decl *D, clang::SourceLocation &SL) {
  if (D) {
    SL = D->getBeginLoc();
    return true;
  } else {
    return false;
  }
}

bool
XMLRecursiveASTVisitor::SourceLocForNestedNameSpecifier(clang::NestedNameSpecifier *NS, clang::SourceLocation &SL) {
  (void)NS;
  (void)SL;
  return false;
}

bool
XMLRecursiveASTVisitor::SourceLocForNestedNameSpecifierLoc(clang::NestedNameSpecifierLoc NL, clang::SourceLocation &SL) {
  SL = NL.getLocalBeginLoc();
  return true;
}

bool
XMLRecursiveASTVisitor::SourceLocForDeclarationNameInfo(clang::DeclarationNameInfo DN, clang::SourceLocation &SL) {
  SL = DN.getBeginLoc();
  return true;
}

bool
XMLRecursiveASTVisitor::SourceLocForTemplateName(clang::TemplateName TN, clang::SourceLocation &SL) {
  (void)TN;
  (void)SL;
  return false;
}

bool
XMLRecursiveASTVisitor::SourceLocForTemplateArgument(const clang::TemplateArgument &TA, clang::SourceLocation &SL) {
  (void)TA;
  (void)SL;
  return false;
}

bool
XMLRecursiveASTVisitor::SourceLocForTemplateArgumentLoc(const clang::TemplateArgumentLoc &TL, clang::SourceLocation &SL) {
  (void)TL;
  (void)SL;
  return false;
}


bool
XMLRecursiveASTVisitor::SourceLocForConstructorInitializer(
							   clang::CXXCtorInitializer *CI, clang::SourceLocation &SL) {
  (void)CI;
  (void)SL;
  return false;
}


///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
