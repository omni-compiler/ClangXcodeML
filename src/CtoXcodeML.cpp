#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/DeclVisitor.h"
#include "clang/AST/TypeLocVisitor.h"
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
#include <tuple>

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;


static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::OptionCategory C2XcodeMLCategory("CtoXcodeML options");
static std::unique_ptr<opt::OptTable> Options(createDriverOptTable());
static cl::opt<bool>
optEmitSourceFileName("file", cl::desc("emit 'file'"),
                      cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
optEmitSourceLineNo("lineno", cl::desc("emit 'lineno'"),
                    cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
optEmitSourceColumn("column", cl::desc("emit 'column'"),
                    cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
optEmitSourceRange("range", cl::desc("emit 'range'"),
                   cl::cat(C2XcodeMLCategory));

class XcodeMlTypeTableVisitor
    : public ConstDeclVisitor<XcodeMlTypeTableVisitor, xmlNodePtr> {
private:
    const ASTContext &astContext; // used for getting additional AST info

public:
    XcodeMlTypeTableVisitor() = delete;
    explicit XcodeMlTypeTableVisitor(const ASTContext &astContext_)
        : astContext(astContext_) {}
};

class XcodeMlSymbolsVisitor
    : public ConstDeclVisitor<XcodeMlSymbolsVisitor, xmlNodePtr> {
private:
    const ASTContext &astContext; // used for getting additional AST info

public:
    XcodeMlSymbolsVisitor() = delete;
    explicit XcodeMlSymbolsVisitor(const ASTContext &astContext_)
        : astContext(astContext_) {}
};

class XcodeMlDeclarationsVisitor
    : public ConstDeclVisitor<XcodeMlDeclarationsVisitor, xmlNodePtr> {
private:
    const ASTContext &astContext; // used for getting additional AST info

    xmlNodePtr newNode(const char *name, SourceLocation loc) {
        xmlNodePtr curNode = xmlNewNode(nullptr, BAD_CAST name);
        FullSourceLoc fullloc = astContext.getFullLoc(loc);
        if (fullloc.isValid()) {
            PresumedLoc ploc = fullloc.getManager().getPresumedLoc(fullloc);

            if (optEmitSourceColumn) {
                xmlChar columnStr[BUFSIZ];
                xmlStrPrintf(columnStr, BUFSIZ, BAD_CAST "%d", ploc.getColumn());
                xmlNewProp(curNode, BAD_CAST "column", columnStr);
            }

            if (optEmitSourceLineNo) {
                xmlChar linenoStr[BUFSIZ];
                xmlStrPrintf(linenoStr, BUFSIZ, BAD_CAST "%d", ploc.getLine());
                xmlNewProp(curNode, BAD_CAST "lineno", linenoStr);
            }

            if (optEmitSourceFileName) {
                xmlNewProp(curNode, BAD_CAST "file", BAD_CAST ploc.getFilename());
            }
        }
        return curNode;
    }

public:
    XcodeMlDeclarationsVisitor() = delete;
    explicit XcodeMlDeclarationsVisitor(const ASTContext &astContext_)
        : astContext(astContext_) {}

    xmlNodePtr VisitDecl(const Decl *decl) {
        xmlNodePtr curNode = newNode(decl->getDeclKindName(),
                                     decl->getLocation());
        const DeclContext *declContext = dyn_cast<DeclContext>(decl);
        if (declContext) {
            for (auto i = declContext->decls_begin();
                 i != declContext->decls_end();
                 ++i) {
                xmlAddChild(curNode, Visit(*i));
            }
        }
        return curNode;
    }
#if 0
    xmlNodePtr VisitFunctionDecl(const FunctionDecl *decl) {
        xmlNodePtr curNode = newNode("HOGEE",
                                     decl->getLocation());
        const DeclContext *declContext = dyn_cast<DeclContext>(decl);
        if (declContext) {
            for (auto i = declContext->decls_begin();
                 i != declContext->decls_end();
                 ++i) {
                xmlAddChild(curNode, Visit(*i));
            }
        }
        return curNode;
    }
#endif

#if 0
    xmlNodePtr VisitExpr(const Expr *expr) {
        return newChild("Expr", expr->getExprLoc());
    }

    xmlNodePtr VisitStmt(const Stmt *stmt) {
        errs() << "VisitStmt\n";
        //return newChild("Stmt", SourceLocation());
        return nullptr;
    }
#endif
};

class XcodeMlASTConsumer : public ASTConsumer {
    const ASTContext &astContext;
    xmlNodePtr typeTable;
    xmlNodePtr globalSymbols;
    xmlNodePtr globalDeclarations;

public:
    explicit XcodeMlASTConsumer(clang::CompilerInstance &CI, xmlNodePtr rootnode)
        : astContext(CI.getASTContext()),
          typeTable(xmlNewChild(rootnode, nullptr,
                                BAD_CAST "typeTable", nullptr)),
          globalSymbols(xmlNewChild(rootnode, nullptr,
                                    BAD_CAST "globalSymbols", nullptr)),
          globalDeclarations(xmlNewChild(rootnode, nullptr,
                                         BAD_CAST "globalDeclarations", nullptr)) {}

    virtual bool HandleTopLevelDecl(DeclGroupRef DG) override {
        std::unique_ptr<XcodeMlTypeTableVisitor>typeTableVisitor
            (new XcodeMlTypeTableVisitor(astContext));
        std::unique_ptr<XcodeMlSymbolsVisitor>globalSymbolsVisitor
            (new XcodeMlSymbolsVisitor(astContext));
        std::unique_ptr<XcodeMlDeclarationsVisitor>globalDeclarationsVisitor
            (new XcodeMlDeclarationsVisitor(astContext));

        // a DeclGroupRef may have multiple Decls, so we iterate through each one
        for (Decl *D : DG) {
            xmlAddChild(typeTable, typeTableVisitor->Visit(D));
            xmlAddChild(globalSymbols, globalSymbolsVisitor->Visit(D));
            xmlAddChild(globalDeclarations, globalDeclarationsVisitor->Visit(D));
        }
        return true;
    }
};

class XcodeMlASTDumpAction : public ASTFrontendAction {
private:
    xmlDocPtr xmlDoc;

public:
    bool BeginSourceFileAction(clang::CompilerInstance& CI,
                             StringRef Filename) override {
        xmlDoc = xmlNewDoc(BAD_CAST "1.0");
        xmlNodePtr rootnode
            = xmlNewNode(nullptr, BAD_CAST "XcodeProgram");
        xmlDocSetRootElement(xmlDoc, rootnode);

        char strftimebuf[BUFSIZ];
        time_t t = time(nullptr);

        strftime(strftimebuf, sizeof strftimebuf, "%F %T", localtime(&t));

        xmlNewProp(rootnode, BAD_CAST "source", BAD_CAST Filename.data());
        xmlNewProp(rootnode, BAD_CAST "language", BAD_CAST "C");
        xmlNewProp(rootnode, BAD_CAST "time", BAD_CAST strftimebuf);

        return true;
    };

    virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI,
                                           StringRef file) override {
        return new XcodeMlASTConsumer(CI, xmlDocGetRootElement(xmlDoc));
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
