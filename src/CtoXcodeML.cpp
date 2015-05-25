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

//static cl::opt<bool>
//OptTraverseBinOp("traverse-binop",
//                 cl::desc("traverse BinaryOperator RecursiveASTvisitor"),
//                 cl::cat(C2XcodeMLCategory));
//static cl::opt<bool>
//OptTraverseType("traverse-type",
//                 cl::desc("traverse Type by RecursiveASTvisitor"),
//                 cl::cat(C2XcodeMLCategory));
//static cl::opt<bool>
//OptTraverseDecl("traverse-decl",
//                 cl::desc("traverse Decl by RecursiveASTvisitor"),
//                 cl::cat(C2XcodeMLCategory));

// use CRTP (Curiously Recurring Template Pattern)
template <class Derived> class XcodeMlVisitorBase
    : public RecursiveASTVisitor<Derived> {
protected:
    static const char *visitorName;
    const ASTContext &astContext; // used for getting additional AST info
    const xmlNodePtr rootNode;          // the current root node.
    xmlNodePtr curNode;           // a candidate of the new chlid.
    typedef RecursiveASTVisitor<Derived> Base;

    void newChild(const char *Name) {
        xmlNodeSetName(curNode, BAD_CAST Name);
    }
    void avoidChild() {
        if (curNode != rootNode) {
            xmlFreeNode(curNode);
        }
        curNode = rootNode;
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

    xmlNodePtr newComment(const char *str, xmlNodePtr RN = nullptr) {
        if (!RN) RN = rootNode;
        xmlChar Buf[BUFSIZ];
        xmlStrPrintf(Buf, BUFSIZ,
                     BAD_CAST "%s::%s", BAD_CAST visitorName, BAD_CAST str);
        xmlNodePtr Comment = xmlNewComment(Buf);
        xmlAddChild(RN, Comment);
        return Comment;
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
          curNode(Name ? xmlNewNode(nullptr, BAD_CAST Name) : N) {}
    explicit XcodeMlVisitorBase(const XcodeMlVisitorBase *p,
                                const char *Name = nullptr)
        : astContext(p->astContext),
          rootNode(p->curNode),
          curNode(Name ? xmlNewNode(nullptr, BAD_CAST Name) : p->curNode) {
        if (Name) {
            newComment(Name);
        }
    };
    ~XcodeMlVisitorBase() {
        if (curNode != rootNode) {
            xmlAddChild(rootNode, curNode);
        }
    }

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
#define DECL(CLASS, BASE)                                              \
    bool Traverse##CLASS##Decl(CLASS##Decl *D) {                       \
        newChild("Traverse" #CLASS "Decl");                            \
        return static_cast<Base &>(*this).Traverse##CLASS##Decl(D);    \
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
};
const char *XcodeMlSymbolsVisitor::visitorName = "XSV";

class XcodeMlDeclarationsVisitor
    : public XcodeMlVisitorBase<XcodeMlDeclarationsVisitor> {
protected:
    static const char *visitorName;
public:
    // use base constructors
    using XcodeMlVisitorBase::XcodeMlVisitorBase;

    bool VisitBinaryOperator(BinaryOperator *S) {
        const char *Nam;

        switch (S->getOpcode()) {
        case BO_PtrMemD:   Nam = "zPtrMemD"; break;
        case BO_PtrMemI:   Nam = "zPtrMemI"; break;
        case BO_Mul:       Nam = "zMul"; break;
        case BO_Div:       Nam = "zDiv"; break;
        case BO_Rem:       Nam = "zRem"; break;
        case BO_Add:       Nam = "zAdd"; break;
        case BO_Sub:       Nam = "zSub"; break;
        case BO_Shl:       Nam = "zShl"; break;
        case BO_Shr:       Nam = "zShr"; break;
        case BO_LT:        Nam = "zLT"; break;
        case BO_GT:        Nam = "zGT"; break;
        case BO_LE:        Nam = "zLE"; break;
        case BO_GE:        Nam = "zGE"; break;
        case BO_EQ:        Nam = "zEQ"; break;
        case BO_NE:        Nam = "zNE"; break;
        case BO_And:       Nam = "zAnd"; break;
        case BO_Xor:       Nam = "zXor"; break;
        case BO_Or:        Nam = "zOr"; break;
        case BO_LAnd:      Nam = "zLAnd"; break;
        case BO_LOr:       Nam = "zLOr"; break;
        case BO_Assign:    Nam = "zAssign"; break;
        case BO_Comma:     Nam = "zComma"; break;
        case BO_MulAssign: Nam = "zMulAssign"; break;
        case BO_DivAssign: Nam = "zDivAssign"; break;
        case BO_RemAssign: Nam = "zRemAssign"; break;
        case BO_AddAssign: Nam = "zAddAssign"; break;
        case BO_SubAssign: Nam = "zSubAssign"; break;
        case BO_ShlAssign: Nam = "zShlAssign"; break;
        case BO_ShrAssign: Nam = "zShrAssign"; break;
        case BO_AndAssign: Nam = "zAndAssign"; break;
        case BO_OrAssign:  Nam = "zOrAssign"; break;
        case BO_XorAssign: Nam = "zXorAssign"; break;
        }
        newComment((std::string("VisitBinaryOperator ") + std::string(Nam)).c_str());
        newChild(Nam);
        setLocation(S->getExprLoc());
        //XcodeMlDeclarationsVisitor V(this);
        //V.TraverseStmt(S->getLHS());
        //V.TraverseStmt(S->getRHS());
        //return OptTraverseBinOp;
        return true;
    }

    bool VisitType(const Type *T) {
        newComment("VisitType");
        newChild("zType");
        //XcodeMlDeclarationsVisitor V(this);
        //V.TraverseDecl(/* ??? */)
        //return OptTraverseType;
        return true;
    }

    bool VisitDecl(const Decl *D) {
        const char *Nam = D->getDeclKindName();
        newComment((std::string("VisitDecl ") + std::string(Nam)).c_str());
        newChild((std::string("zDecl") + std::string(Nam)).c_str());
        setLocation(D);

        //const DeclContext *DC = dyn_cast<DeclContext>(D);
        //if (DC) {
        //    for (auto *ChildD : DC->noload_decls()) {
        //        newChild(ChildD->getDeclKindName(), curNode);
        //        setLocation(ChildD);
        //        XcodeMlDeclarationsVisitor V(this);
        //        V.TraverseDecl(ChildD);
        //    }
        //}
        //return OptTraverseDecl;
        return true;
    }

    bool VisitFunctionDecl(const FunctionDecl *D) {
        newComment("VisitFunctionDecl");
        return true;
    }

    bool VisitExpr(const Expr *E) {
        newComment("VisitExpr");
        newChild("zExpr");
        setLocation(E);
        return true;
    }
    bool VisitStmt(const Stmt *S) {
        newComment("VisitStmt");
        newChild("zStmt");
        return true;
    }
};
const char *XcodeMlDeclarationsVisitor::visitorName = "XDV";


class XcodeMlASTConsumer : public ASTConsumer {
    const xmlNodePtr rootNode;

    xmlNodePtr addNewChild(const char *Name) {
        //errs() << "XcodeMlASTConsumer::newChild(" << Name << ")\n";
        return xmlNewChild(rootNode, nullptr, BAD_CAST Name, nullptr);
    }

public:
    explicit XcodeMlASTConsumer(xmlNodePtr N) : rootNode(N) {};

    virtual void HandleTranslationUnit(ASTContext &CXT) override {
        XcodeMlTypeTableVisitor
            TypeTableVisitor(CXT, addNewChild("TypeTable"));
        XcodeMlSymbolsVisitor
            GlobalSymbolsVisitor(CXT, addNewChild("globalSymbols"));
        XcodeMlDeclarationsVisitor
            GlobalDeclarationsVisitor(CXT,
                                      addNewChild("globalDeclarations"));
        Decl *D = CXT.getTranslationUnitDecl();

        TypeTableVisitor.TraverseDecl(D);
        GlobalSymbolsVisitor.TraverseDecl(D);
        GlobalDeclarationsVisitor.TraverseDecl(D);
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
