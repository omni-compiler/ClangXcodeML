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

#include <libxml/tree.h>
#include <time.h>
#include <string>

using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::OptionCategory C2XcodeMLCategory("CtoXcodeML options");
static std::unique_ptr<opt::OptTable> Options(createDriverOptTable());
static cl::opt<bool>
OptEmitSourceFileName("file", cl::desc("emit 'file'"),
                      cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptEmitSourceLineNo("lineno", cl::desc("emit 'lineno'"),
                    cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptEmitSourceColumn("column", cl::desc("emit 'column'"),
                    cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptEmitSourceRange("range", cl::desc("emit 'range'"),
                   cl::cat(C2XcodeMLCategory));

static cl::opt<bool>
OptTraceXTTV("trace-xttv",
             cl::desc("emit traces on XcodeMlTypeTableVisitor"),
             cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptTraceXSV("trace-xsv",
            cl::desc("emit traces on XcodeMlSymbolVisitor"),
            cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptTraceXDV("trace-xdv",
            cl::desc("emit traces on XcodeMlDeclarationsVisitor"),
            cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXTTV("disable-xttv",
               cl::desc("disable XcodeMlTypeTableVisitor"),
               cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXSV("disable-xsv",
              cl::desc("disable XcodeMlSymbolsVisitor"),
              cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableXDV("disable-xdv",
              cl::desc("disable XcodeMlDeclarationsVisitor"),
              cl::cat(C2XcodeMLCategory));

// use CRTP (Curiously Recurring Template Pattern)
template <class Derived> class XcodeMlVisitorBase
    : public RecursiveASTVisitor<Derived> {
protected:
    static const char *visitorName;
    const ASTContext &astContext;
    const xmlNodePtr rootNode;    // the current root node.
    xmlNodePtr curNode;           // a candidate of the new chlid.
    bool addCurNodeAsChildOfRootNode;
    typedef RecursiveASTVisitor<Derived> Base;

    void setName(const char *Name) {
        xmlNodeSetName(curNode, BAD_CAST Name);
    }
    void avoidChild() {
        if (curNode != rootNode) {
            xmlFreeNode(curNode);
        }
        curNode = rootNode;
        addCurNodeAsChildOfRootNode = false;
    }
    void newProp(const char *Name, int Val, xmlNodePtr N = nullptr) {
        if (!N) N = curNode;
        xmlChar Buf[BUFSIZ];
        xmlStrPrintf(Buf, BUFSIZ, BAD_CAST "%d", Val);
        xmlNewProp(N, BAD_CAST Name, Buf);
    }
    void newProp(const char *Name, const char *Val, xmlNodePtr N = nullptr) {
        if (!N) N = curNode;
        xmlNewProp(N, BAD_CAST Name, BAD_CAST Val);
    }

    void newComment(const char *str, xmlNodePtr RN = nullptr) {
        if (!RN) RN = rootNode;
        xmlChar Buf[BUFSIZ];
        const char *VN = static_cast<Derived &>(*this).getVisitorName();
        if (VN) {
            xmlStrPrintf(Buf, BUFSIZ,
                         BAD_CAST "%s::%s", BAD_CAST VN, BAD_CAST str);
            xmlNodePtr Comment = xmlNewComment(Buf);
            xmlAddChild(RN, Comment);
        }
    }

    void setLocation(SourceLocation Loc, xmlNodePtr N = nullptr) {
        if (!N) N = curNode;
        FullSourceLoc FLoc = astContext.getFullLoc(Loc);
        if (FLoc.isValid()) {
            PresumedLoc PLoc = FLoc.getManager().getPresumedLoc(FLoc);

            if (OptEmitSourceColumn) {
                newProp("column", PLoc.getColumn(), N);
            }
            if (OptEmitSourceLineNo) {
                newProp("lineno", PLoc.getLine(), N);
            }
            if (OptEmitSourceFileName) {
                newProp("file", PLoc.getFilename(), N);
            }
        }
    }
    void setLocation(const Decl *D, xmlNodePtr N = nullptr) {
        setLocation(D->getLocation(), N);
    }
    void setLocation(const Expr *E, xmlNodePtr N = nullptr) {
        setLocation(E->getExprLoc(), N);
    }

public:
    XcodeMlVisitorBase() = delete;
    XcodeMlVisitorBase(const XcodeMlVisitorBase&) = delete;
    XcodeMlVisitorBase(XcodeMlVisitorBase&&) = delete;
    XcodeMlVisitorBase& operator =(const XcodeMlVisitorBase&) = delete;
    XcodeMlVisitorBase& operator =(XcodeMlVisitorBase&&) = delete;

    explicit XcodeMlVisitorBase(const ASTContext &CXT, xmlNodePtr N,
                                const char *Name = nullptr)
        : astContext(CXT), rootNode(N),
          curNode(Name ? xmlNewNode(nullptr, BAD_CAST Name) : N),
          addCurNodeAsChildOfRootNode(true) {}
    explicit XcodeMlVisitorBase(const XcodeMlVisitorBase *p,
                                const char *Name = nullptr)
        : astContext(p->astContext),
          rootNode(p->curNode),
          curNode(Name ? xmlNewNode(nullptr, BAD_CAST Name) : p->curNode),
          addCurNodeAsChildOfRootNode(true) {
        if (Name) {
            newComment(Name);
        }
    };
    ~XcodeMlVisitorBase() {
        if (addCurNodeAsChildOfRootNode) {
            xmlAddChild(rootNode, curNode);
        }
    }

    const char *getVisitorName() const { return nullptr; }

    bool shouldVisitImplicitCode() const { return true; }

    // avoid data-recursion (force traversing with hardware stack)
    bool shouldUseDataRecursionFor(Stmt *S) const { return false; }

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT)                               \
    bool Traverse##CLASS(CLASS *S) {                      \
        Derived V(this, "Traverse" #CLASS);               \
        return static_cast<Base &>(V).Traverse##CLASS(S); \
    }
#include "clang/AST/StmtNodes.inc"

#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                       \
    bool Traverse##CLASS##Type(CLASS##Type *T) {                \
        Derived V(this, "Traverse" #CLASS "Type");              \
        return static_cast<Base &>(V).Traverse##CLASS##Type(T); \
    }
#include "clang/AST/TypeNodes.def"

    bool TraverseDecl(Decl *D) {
        Derived V(this, "TraverseDecl");
        return static_cast<Base &>(V).TraverseDecl(D);
    }
#define ABSTRACT_DECL(DECL)
#define DECL(CLASS, BASE)                                           \
    bool Traverse##CLASS##Decl(CLASS##Decl *D) {                    \
        setName("Traverse" #CLASS "Decl");                          \
        return static_cast<Base &>(*this).Traverse##CLASS##Decl(D); \
    }
#include "clang/AST/DeclNodes.inc"
};


template <class Derived>
const char *XcodeMlVisitorBase<Derived>::visitorName = "XVBase";

class XcodeMlTypeTableVisitor
    : public XcodeMlVisitorBase<XcodeMlTypeTableVisitor> {
protected:
    static const char *visitorName;
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const {
        return OptTraceXTTV ? visitorName : nullptr;
    }

    bool VisitStmt(const Stmt *S) {
        newComment("VisitStmt");
        avoidChild();
        return true;
    }
    bool VisitType(const Type *D) {
        newComment("VisitType");
        return true;
    }
    bool VisitDecl(const Decl *D) {
        newComment("VisitDecl");
        avoidChild();
        return true;
    }
};
const char *XcodeMlTypeTableVisitor::visitorName = "XTTV";

class XcodeMlSymbolsVisitor
    : public XcodeMlVisitorBase<XcodeMlSymbolsVisitor> {
protected:
    static const char *visitorName;
public:
    // use base constructors
    using XcodeMlVisitorBase<XcodeMlSymbolsVisitor>::XcodeMlVisitorBase;

    const char *getVisitorName() const {
        return OptTraceXSV ? visitorName : nullptr;
    }
};
const char *XcodeMlSymbolsVisitor::visitorName = "XSV";

class XcodeMlDeclarationsVisitor
    : public XcodeMlVisitorBase<XcodeMlDeclarationsVisitor> {
protected:
    static const char *visitorName;
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    const char *getVisitorName() const {
        return OptTraceXDV ? visitorName : nullptr;
    }

    bool VisitUnaryOperator(const UnaryOperator *UnOp) {
        // XcodeML-C-0.9J.pdf 7.2(varAddr), 7.3(pointerRef), 7.8, 7.11
        const char *Nam;

        switch (UnOp->getOpcode()) {
        case UO_PostInc:   Nam = "postIncrExpr"; break;
        case UO_PostDec:   Nam = "postDecrExpr"; break;
        case UO_PreInc:    Nam = "preIncrExpr"; break;
        case UO_PreDec:    Nam = "preDecrExpr"; break;
        case UO_AddrOf:    Nam = "varAddr"; break;
        case UO_Deref:     Nam = "pointerRef"; break;
        case UO_Plus:      Nam = "UNDEF_UO_Plus"; break;
        case UO_Minus:     Nam = "unaryMinusExpr"; break;
        case UO_Not:       Nam = "bitNotExpr"; break;
        case UO_LNot:      Nam = "logNotExpr"; break;
        case UO_Real:      Nam = "UNDEF_UO_Real"; break;
        case UO_Imag:      Nam = "UNDEF_UO_Imag"; break;
        case UO_Extension: Nam = "UNDEF_UO_Extension"; break;
        }
        newComment((std::string("VisitUnaryOperator ") + std::string(Nam)).c_str());
        setName(Nam);
        setLocation(UnOp);
        return true;
    }

    bool VisitBinaryOperator(const BinaryOperator *BinOp) {
        // XcodeML-C-0.9J.pdf: 7.6(assignExpr), 7.7, 7.10(commmaExpr)
        const char *Nam;

        switch (BinOp->getOpcode()) {
        case BO_PtrMemD:   Nam = "UNDEF_BO_PtrMemD"; break;
        case BO_PtrMemI:   Nam = "UNDEF_BO_PtrMemI"; break;
        case BO_Mul:       Nam = "mulExpr"; break;
        case BO_Div:       Nam = "divExpr"; break;
        case BO_Rem:       Nam = "modExpr"; break;
        case BO_Add:       Nam = "plusExpr"; break;
        case BO_Sub:       Nam = "minusExpr"; break;
        case BO_Shl:       Nam = "LshiftExpr"; break;
        case BO_Shr:       Nam = "RshiftExpr"; break;
        case BO_LT:        Nam = "logLTExpr"; break;
        case BO_GT:        Nam = "logGTExpr"; break;
        case BO_LE:        Nam = "logLEExpr"; break;
        case BO_GE:        Nam = "logGEExpr"; break;
        case BO_EQ:        Nam = "ogEQExpr"; break;   // og? l is missing?
        case BO_NE:        Nam = "logNEQExpr"; break;
        case BO_And:       Nam = "bitAndExpr"; break;
        case BO_Xor:       Nam = "bitXorExpr"; break;
        case BO_Or:        Nam = "bitOrExpr"; break;
        case BO_LAnd:      Nam = "logAndExpr"; break;
        case BO_LOr:       Nam = "logOrExpr"; break;
        case BO_Assign:    Nam = "assignExpr"; break;
        case BO_Comma:     Nam = "commaExpr"; break;
        case BO_MulAssign: Nam = "asgMulExpr"; break;
        case BO_DivAssign: Nam = "asgDivExpr"; break;
        case BO_RemAssign: Nam = "asgModExpr"; break;
        case BO_AddAssign: Nam = "asgPlusExpr"; break;
        case BO_SubAssign: Nam = "asgMinusExpr"; break;
        case BO_ShlAssign: Nam = "asgLshiftExpr"; break;
        case BO_ShrAssign: Nam = "asgRshiftExpr"; break;
        case BO_AndAssign: Nam = "asgBitAndExpr"; break;
        case BO_OrAssign:  Nam = "asgBitOrExpr"; break;
        case BO_XorAssign: Nam = "asgBitXorExpr"; break;
        }
        newComment((std::string("VisitBinaryOperator ") + std::string(Nam)).c_str());
        setName(Nam);
        setLocation(BinOp);
        return true;
    }

    bool VisitBinaryConditionalOperator(const BinaryConditionalOperator *ConOp) {
        setName("UNDEF_BinaryConditionalOperator");
        setLocation(ConOp);
        return true;
    }
    bool VisitConditionalOperator(const ConditionalOperator *ConOp) {
        // XcodeML-C-0.9J.pdf 7.13
        setName("condExpr");
        setLocation(ConOp);
        return true;
    }
    bool VisitCompoundStmt(CompoundStmt *S) {
        // XcodeML-C-0.9J.pdf 6.2
        XcodeMlSymbolsVisitor SV(this->astContext, this->curNode, "symbols");
        if (!OptDisableXSV) {
            SV.TraverseCompoundStmt(S);
        }
        //XcodeMlDeclarationsVisitor DV(this->astContext, this->curNode,
        //                              "Declarations");
        //DV.TraverseCompoundStmt(S);

        setName("compoundStatement");
        return true;
    }


    bool VisitExpr(const Expr *E) {
        newComment("VisitExpr");
        //setName("zExpr");
        //setLocation(E);
        return true;
    }
    bool VisitStmt(const Stmt *S) {
        newComment("VisitStmt");
        //setName("zStmt");
        return true;
    }

    bool VisitType(const Type *T) {
        newComment("VisitType");
        //setName("zType");
        return true;
    }

    bool VisitDecl(const Decl *D) {
        const char *Nam = D->getDeclKindName();
        newComment((std::string("VisitDecl ") + std::string(Nam)).c_str());
        //setName((std::string("Decl") + std::string(Nam)).c_str());
        setLocation(D);
        return true;
    }

    bool VisitFunctionDecl(FunctionDecl *D) {
        // XcodeML-C-0.9J.pdf 5.1
        newComment("VisitFunctionDecl");
        setName("functionDefinition");
        DeclarationNameInfo DNI = D->getNameInfo();
        //setLocation(DNI.getLoc());

        const char *FuncName = DNI.getName().getAsString().c_str();
        xmlNewChild(curNode, nullptr, BAD_CAST "name", BAD_CAST FuncName);

        XcodeMlSymbolsVisitor SV(this->astContext, this->curNode, "symbols");
        if (!OptDisableXSV) {
            SV.TraverseFunctionDecl(D);
        }

        // should be handle <param> properly

        xmlAddChild(rootNode, curNode);
        curNode = xmlNewChild(curNode, nullptr, BAD_CAST "body", nullptr);
        addCurNodeAsChildOfRootNode = false;
        return true;
    }
};
const char *XcodeMlDeclarationsVisitor::visitorName = "XDV";


class XcodeMlASTConsumer : public ASTConsumer {
    const xmlNodePtr rootNode;

    xmlNodePtr addNewChild(const char *Name) {
        return xmlNewChild(rootNode, nullptr, BAD_CAST Name, nullptr);
    }

public:
    explicit XcodeMlASTConsumer(xmlNodePtr N) : rootNode(N) {};

    virtual void HandleTranslationUnit(ASTContext &CXT) override {
        XcodeMlTypeTableVisitor TTV(CXT, addNewChild("TypeTable"));
        XcodeMlSymbolsVisitor SV(CXT, addNewChild("globalSymbols"));
        XcodeMlDeclarationsVisitor DV(CXT, addNewChild("globalDeclarations"));

        Decl *D = CXT.getTranslationUnitDecl();

        if (!OptDisableXTTV) {
            TTV.TraverseDecl(D);
        }
        if (!OptDisableXSV) {
            SV.TraverseDecl(D);
        }
        if (!OptDisableXDV) {
            DV.TraverseDecl(D);
        }
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
        return new XcodeMlASTConsumer(xmlDocGetRootElement(xmlDoc));
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
