#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Option/OptTable.h"
#include "llvm/Support/Signals.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <time.h>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;


static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::OptionCategory C2XcodeMLCategory("CtoXcodeML options");
static std::unique_ptr<opt::OptTable> Options(createDriverOptTable());
static cl::opt<bool>
EmitSourceFileName("file", cl::desc("emit 'file'"),
                   cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
EmitSourceLineNo("lineno", cl::desc("emit 'lineno'"),
                 cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
EmitSourceColumn("column", cl::desc("emit 'column'"),
                 cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
EmitSourceRange("range", cl::desc("emit 'range'"),
                cl::cat(C2XcodeMLCategory));

class XcodeMlTypeTableVisitor
    : public RecursiveASTVisitor<XcodeMlTypeTableVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info
    xmlDocPtr xmlDoc;
    xmlNodePtr typeTable;

public:
    XcodeMlTypeTableVisitor(CompilerInstance &CI, xmlDocPtr xmlDoc_,
                            xmlNodePtr typeTable_)
        : astContext(&(CI.getASTContext())),
          xmlDoc(xmlDoc_),
          typeTable(typeTable_) {}
};

class XcodeMlSymbolsVisitor
    : public RecursiveASTVisitor<XcodeMlSymbolsVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info
    xmlDocPtr xmlDoc;
    xmlNodePtr symbolTable;

public:
    XcodeMlSymbolsVisitor(CompilerInstance &CI, xmlDocPtr xmlDoc_,
                          xmlNodePtr symbolTable_)
        : astContext(&(CI.getASTContext())),
          xmlDoc(xmlDoc_),
          symbolTable(symbolTable_) {}
};

class XcodeMlDeclarationsVisitor
    : public RecursiveASTVisitor<XcodeMlDeclarationsVisitor> {
private:
    ASTContext *astContext; // used for getting additional AST info
    xmlDocPtr xmlDoc;
    xmlNodePtr declTable;

public:
    XcodeMlDeclarationsVisitor(CompilerInstance &CI, xmlDocPtr xmlDoc_,
                               xmlNodePtr declTable_)
        : astContext(&(CI.getASTContext())),
          xmlDoc(xmlDoc_),
          declTable(declTable_) {}

#if 0
    virtual bool VisitFunctionDecl(FunctionDecl *func) {
        numFunctions++;
        string funcName = func->getNameInfo().getName().getAsString();

        errs() << "found function def: " << ":" << funcName << "\n";

        return true;
    }

    virtual bool VisitStmt(Stmt *st) {
        if (ReturnStmt *ret = dyn_cast<ReturnStmt>(st)) {
            rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
            errs() << "** Rewrote ReturnStmt\n";
        }
        if (CallExpr *call = dyn_cast<CallExpr>(st)) {
            rewriter.ReplaceText(call->getLocStart(), 7, "add5");
            errs() << "** Rewrote function call\n";
        }
        return true;
    }

    virtual bool VisitReturnStmt(ReturnStmt *ret) {
        rewriter.ReplaceText(ret->getRetValue()->getLocStart(), 6, "val");
        errs() << "** Rewrote ReturnStmt\n";
        return true;
    }
    virtual bool VisitCallExpr(CallExpr *call) {
        rewriter.ReplaceText(call->getLocStart(), 7, "add5");
        errs() << "** Rewrote function call\n";
        return true;
    }
#endif
};

class XcodeMlASTConsumer : public ASTConsumer {
    xmlDocPtr xmlDoc;
    XcodeMlTypeTableVisitor *typeTableVisitor;
    XcodeMlSymbolsVisitor *globalSymbolsVisitor;
    XcodeMlDeclarationsVisitor *globalDeclarationsVisitor;

public:
    explicit XcodeMlASTConsumer(xmlDocPtr xmlDoc_,
                                XcodeMlTypeTableVisitor *typeTableVisitor_,
                                XcodeMlSymbolsVisitor *globalSymbolsVisitor_,
                                XcodeMlDeclarationsVisitor *globalDeclarationsVisitor_)
        : xmlDoc(xmlDoc_),
          typeTableVisitor(typeTableVisitor_),
          globalSymbolsVisitor(globalSymbolsVisitor_),
          globalDeclarationsVisitor(globalDeclarationsVisitor_) {};

    virtual void HandleTranslationUnit(ASTContext &Context) override {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
           a single Decl that collectively represents the entire source file */
        typeTableVisitor->TraverseDecl(Context.getTranslationUnitDecl());
        globalSymbolsVisitor->TraverseDecl(Context.getTranslationUnitDecl());
        globalDeclarationsVisitor->TraverseDecl(Context.getTranslationUnitDecl());
    }
#if 0
    virtual bool HandleTopLevelDecl(DeclGroupRef DG) override {
        // a DeclGroupRef may have multiple Decls, so we iterate through each one
        for (DeclGroupRef::iterator i = DG.begin(), e = DG.end(); i != e; i++) {
            Decl *D = *i;
            visitor->TraverseDecl(D); // recursively visit each AST node in Decl "D"
        }
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
            = xmlNewNode(NULL, BAD_CAST "XcodeProgram");
        xmlDocSetRootElement(xmlDoc, rootnode);

        char strftimebuf[BUFSIZ];
        time_t t = time(NULL);

        strftime(strftimebuf, sizeof strftimebuf, "%F %T", localtime(&t));

        xmlNewProp(rootnode, BAD_CAST "source", BAD_CAST Filename.data());
        xmlNewProp(rootnode, BAD_CAST "language", BAD_CAST "C");
        xmlNewProp(rootnode, BAD_CAST "time", BAD_CAST strftimebuf);

        return true;
    };

    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI,
                                           StringRef file) override {
        xmlNodePtr rootnode = xmlDocGetRootElement(xmlDoc);

        xmlNodePtr typeTable
            = xmlNewChild(rootnode, NULL, BAD_CAST "typeTable", NULL);
        xmlNodePtr globalSymbols
            = xmlNewChild(rootnode, NULL, BAD_CAST "globalSymbols", NULL);
        xmlNodePtr globalDeclarations
            = xmlNewChild(rootnode, NULL, BAD_CAST "globalDeclarations", NULL);
        XcodeMlTypeTableVisitor *typeTableVisitor
            = new XcodeMlTypeTableVisitor(CI, xmlDoc, typeTable);
        XcodeMlSymbolsVisitor *globalSymbolsVisitor
            = new XcodeMlSymbolsVisitor(CI, xmlDoc, globalSymbols);
        XcodeMlDeclarationsVisitor *globalDeclarationsVisitor
            = new XcodeMlDeclarationsVisitor(CI, xmlDoc, globalDeclarations);

        return new XcodeMlASTConsumer(xmlDoc,
                                      typeTableVisitor,
                                      globalSymbolsVisitor,
                                      globalDeclarationsVisitor);
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
    Tool.setArgumentsAdjuster(new clang::tooling::ClangSyntaxOnlyAdjuster());

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
