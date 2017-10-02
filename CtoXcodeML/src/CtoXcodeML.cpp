#include "XcodeMlVisitorBase.h"

#include "SymbolsVisitor.h"
#include "TypeTableVisitor.h"
#include "DeclarationsVisitor.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Driver/Options.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/Signals.h"

#include <libxml/xmlsave.h>
#include <time.h>
#include <string>

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static std::unique_ptr<opt::OptTable> Options(createDriverOptTable());

class XcodeMlASTConsumer : public ASTConsumer {
  xmlNodePtr rootNode;

public:
  explicit XcodeMlASTConsumer(xmlNodePtr N) : rootNode(N){};

  virtual void
  HandleTranslationUnit(ASTContext &CXT) override {
    MangleContext *MC = CXT.createMangleContext();
    InheritanceInfo inheritanceinfo;
    InheritanceInfo *II = &inheritanceinfo;
    TypeTableInfo typetableinfo(MC, II);
    TypeTableInfo *TTI = &typetableinfo;
    TypeTableVisitor TTV(MC, rootNode, "typeTable", TTI);
    SymbolsVisitor SV(MC, rootNode, "globalSymbols", TTI);
    DeclarationsVisitor DV(MC, rootNode, "globalDeclarations", TTI);
    Decl *D = CXT.getTranslationUnitDecl();

    TTV.TraverseDecl(D);
    SV.TraverseDecl(D);
    DV.TraverseDecl(D);
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
  bool
  BeginSourceFileAction(
      clang::CompilerInstance &CI, StringRef Filename) override {
    (void)CI; // suppress warnings
    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr rootnode = xmlNewNode(nullptr, BAD_CAST "XcodeProgram");
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
    (void)CI; // suppress warnings
    (void)file; // suppress warnings

    std::unique_ptr<ASTConsumer> C(
        new XcodeMlASTConsumer(xmlDocGetRootElement(xmlDoc)));
    return C;
  }

  void
  EndSourceFileAction(void) override {
    // int saveopt = XML_SAVE_FORMAT | XML_SAVE_NO_EMPTY;
    int saveopt = XML_SAVE_FORMAT;
    xmlSaveCtxtPtr ctxt = xmlSaveToFilename("-", "UTF-8", saveopt);
    xmlSaveDoc(ctxt, xmlDoc);
    xmlSaveClose(ctxt);
    xmlFreeDoc(xmlDoc);
  }
};

int
main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal();
  CommonOptionsParser OptionsParser(argc, argv, C2XcodeMLCategory);
  ClangTool Tool(
      OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
  Tool.appendArgumentsAdjuster(clang::tooling::getClangSyntaxOnlyAdjuster());

#if 0
    errs() << "sizeof(XcodeMlVisitorBaseImpl)=" << sizeof(XcodeMlVisitorBaseImpl) << "\n";
    errs() << "sizeof(TypeTableVisitor)=" << sizeof(TypeTableVisitor) << "\n";
    errs() << "sizeof(SymbolsVisitor)=" << sizeof(SymbolsVisitor) << "\n";
    errs() << "sizeof(DeclarationsVisitor)=" << sizeof(DeclarationsVisitor) << "\n";
#endif

  std::unique_ptr<FrontendActionFactory> FrontendFactory =
      newFrontendActionFactory<XcodeMlASTDumpAction>();
  return Tool.run(FrontendFactory.get());
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
