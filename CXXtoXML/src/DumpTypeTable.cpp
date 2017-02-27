#include "XMLVisitorBase.h"

#include "TypeTableVisitor.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/Signals.h"
#include "DumpTypeTable.h"

#include <libxml/xmlsave.h>
#include <string>

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

void
TypeTableDumpASTConsumer::HandleTranslationUnit(ASTContext & CXT) {
  MangleContext *MC = CXT.createMangleContext();
  InheritanceInfo inheritanceinfo;
  InheritanceInfo *II = &inheritanceinfo;
  TypeTableInfo typetableinfo(MC, II);
  TypeTableInfo *TTI = &typetableinfo;
  Decl *D = CXT.getTranslationUnitDecl();

  TTI->dump();
}


bool TypeTableDumpAction::BeginSourceFileAction(
    clang::CompilerInstance&,
    StringRef)
{
  dummyRoot = xmlNewNode(nullptr, BAD_CAST "Program");
  return true;
}

std::unique_ptr<ASTConsumer>
TypeTableDumpAction::CreateASTConsumer(
    CompilerInstance &,
    StringRef)
{
  std::unique_ptr<ASTConsumer> C(
      new TypeTableDumpASTConsumer(dummyRoot));
  return C;
}


void TypeTableDumpAction::EndSourceFileAction() {
    xmlFreeNode(dummyRoot);
}
