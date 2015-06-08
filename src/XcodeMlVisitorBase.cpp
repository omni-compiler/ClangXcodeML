#include "XcodeMlVisitorBase.h"

#include "clang/Driver/Options.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include <type_traits>

using namespace llvm;

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
OptTraceRAV("trace-rav", cl::desc("trace Recursive AST Visitor"),
            cl::cat(C2XcodeMLCategory));

// implement my RecursiveASTVisitor (which uses CRTP)
class XcodeMlRAV : public RecursiveASTVisitor<XcodeMlRAV>,
                   public RAVBidirBridge {
private:
    typedef RecursiveASTVisitor<XcodeMlRAV> RAV;
public:
    XcodeMlRAV() = delete;
    XcodeMlRAV(const XcodeMlRAV&) = delete;
    XcodeMlRAV(XcodeMlRAV&&) = delete;
    XcodeMlRAV& operator =(const XcodeMlRAV&) = delete;
    XcodeMlRAV& operator =(XcodeMlRAV&&) = delete;

    explicit XcodeMlRAV(RAVBidirBridge *otherside)
        : RAVBidirBridge(otherside) {};

#define DISPATCHER(NAME, TYPE)                              \
    bool Traverse##NAME(TYPE S) {                           \
        const char *VN = otherside->getVisitorName();       \
        if (OptTraceRAV && VN) {                            \
            errs() << VN << "::       Traverse" #NAME "\n"; \
        }                                                   \
        return otherside->Bridge##NAME(S);                  \
    }                                                       \
    bool Bridge##NAME(TYPE S) override {                    \
        const char *VN = otherside->getVisitorName();       \
        if (OptTraceRAV && VN) {                            \
            errs() << VN << "::BridgeTraverse" #NAME "\n";  \
        }                                                   \
        return static_cast<RAV*>(this)->Traverse##NAME(S);  \
    }

    DISPATCHER(Stmt, Stmt *);
    DISPATCHER(Type, QualType);
    DISPATCHER(TypeLoc, TypeLoc);
    DISPATCHER(Attr, Attr *);
    DISPATCHER(Decl, Decl *);
    DISPATCHER(NestedNameSpecifier, NestedNameSpecifier *);
    DISPATCHER(NestedNameSpecifierLoc, NestedNameSpecifierLoc);
    DISPATCHER(DeclarationNameInfo, DeclarationNameInfo);
    DISPATCHER(TemplateName, TemplateName);
    DISPATCHER(TemplateArgument, const TemplateArgument &);
    DISPATCHER(TemplateArgumentLoc, const TemplateArgumentLoc &);
    DISPATCHER(ConstructorInitializer, CXXCtorInitializer *);

    const char *getVisitorName() const override { return "RAV"; }
};
class RAVpoolSizeChecker {
    static_assert(sizeof(XcodeMlRAV)
                  <= sizeof(XcodeMlVisitorBaseImpl::RAVpool),
                  "XcodeMlVisitorBaseImpl::RAVpool is too small");
};

// implementation of XcodeMlVisitorBaseImpl

XcodeMlVisitorBaseImpl::
XcodeMlVisitorBaseImpl(const ASTContext &CXT,
                       xmlNodePtr RootNode,
                       xmlNodePtr CurNode)
    : RAVBidirBridge(new(RAVpool) XcodeMlRAV(this)),
      astContext(CXT), rootNode(RootNode), curNode(CurNode),
      isLocationAlreadySet(true) {};

void XcodeMlVisitorBaseImpl::setName(const char *Name) {
    xmlNodeSetName(curNode, BAD_CAST Name);
}

void XcodeMlVisitorBaseImpl::newProp(const char *Name, int Val, xmlNodePtr N) {
    if (!N) N = curNode;
    xmlChar Buf[BUFSIZ];
    xmlStrPrintf(Buf, BUFSIZ, BAD_CAST "%d", Val);
    xmlNewProp(N, BAD_CAST Name, Buf);
}

void XcodeMlVisitorBaseImpl::newProp(const char *Name, const char *Val,
                                 xmlNodePtr N) {
    if (!N) N = curNode;
    xmlNewProp(N, BAD_CAST Name, BAD_CAST Val);
}

void XcodeMlVisitorBaseImpl::newComment(const xmlChar *str, xmlNodePtr RN) {
    if (!RN) RN = rootNode;
    xmlChar Buf[BUFSIZ];
    const char *VN = getVisitorName();
    if (VN) {
        xmlStrPrintf(Buf, BUFSIZ,
                     BAD_CAST "%s::%s", BAD_CAST VN, str);
        xmlNodePtr Comment = xmlNewComment(Buf);
        xmlAddChild(RN, Comment);
        //errs() << (const char *)Buf << "\n";
    }
}

void XcodeMlVisitorBaseImpl::newComment(const char *str, xmlNodePtr RN) {
    newComment(BAD_CAST str, RN);
}

void XcodeMlVisitorBaseImpl::setLocation(SourceLocation Loc, xmlNodePtr N) {
    if (isLocationAlreadySet)
        return;
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
    isLocationAlreadySet = true;
}

void XcodeMlVisitorBaseImpl::setLocation(const Decl *D, xmlNodePtr N) {
    setLocation(D->getLocation(), N);
}

void XcodeMlVisitorBaseImpl::setLocation(const Expr *E, xmlNodePtr N) {
    setLocation(E->getExprLoc(), N);
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
