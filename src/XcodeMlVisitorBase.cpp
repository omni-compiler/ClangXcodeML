#include "clang/Driver/Options.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Option/OptTable.h"

#include <libxml/tree.h>
#include <time.h>
#include <string>

#include "XcodeMlVisitorBase.h"


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

// implementation of XcodeMlVisitorBaseImpl

XcodeMlVisitorBaseImpl::
XcodeMlVisitorBaseImpl(const ASTContext &CXT, xmlNodePtr N, const char *Name)
    : XcodeMlVisitorBidirectionalBridge(&RAV),
      RAV(this), astContext(CXT), rootNode(N),
      curNode(Name ? xmlNewNode(nullptr, BAD_CAST Name) : N),
      isLocationAlreadySet(false) {
    // do nothing
}

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

// implementation of XcodeMlRAV

#define ABSTRACT_STMT(STMT)
#define STMT(CLASS, PARENT)                                        \
    bool XcodeMlRAV::Traverse##CLASS(CLASS *S) {                   \
        const char *VN = otherside->getVisitorName();              \
        if (OptTraceRAV && VN) {                                   \
            errs() << VN << "::       Traverse" #CLASS "\n";       \
        }                                                          \
        return otherside->XcodeMlTraverse##CLASS(S);               \
    }                                                              \
    bool XcodeMlRAV::XcodeMlTraverse##CLASS(CLASS *S) {            \
        const char *VN = otherside->getVisitorName();              \
        if (OptTraceRAV && VN) {                                   \
            errs() << VN << "::XcodeMlTraverse" #CLASS "\n";       \
        }                                                          \
        return static_cast<RAV*>(this)->Traverse##CLASS(S);        \
    }
#include "clang/AST/StmtNodes.inc"

#define ABSTRACT_TYPE(CLASS, BASE)
#define TYPE(CLASS, BASE)                                            \
    bool XcodeMlRAV::Traverse##CLASS##Type(CLASS##Type *T) {         \
        const char *VN = otherside->getVisitorName();              \
        if (OptTraceRAV && VN) {                                     \
            errs() << VN << "::       Traverse" #CLASS "Type\n";     \
        }                                                            \
        return otherside->XcodeMlTraverse##CLASS##Type(T);           \
    }                                                                \
    bool XcodeMlRAV::XcodeMlTraverse##CLASS##Type(CLASS##Type *T) {  \
        const char *VN = otherside->getVisitorName();              \
        if (OptTraceRAV && VN) {                                     \
            errs() << VN << "::XcodeMlTraverse" #CLASS "Type\n";     \
        }                                                            \
        return static_cast<RAV*>(this)->Traverse##CLASS##Type(T);    \
    }
#include "clang/AST/TypeNodes.def"

bool XcodeMlRAV::TraverseDecl(Decl *D) {
    const char *VN = otherside->getVisitorName();
    if (OptTraceRAV && VN) {
        errs() << VN << "::       TraverseDecl\n";
    }
    return otherside->XcodeMlTraverseDecl(D);
}
bool XcodeMlRAV::XcodeMlTraverseDecl(Decl *D) {
    const char *VN = otherside->getVisitorName();
    if (OptTraceRAV && VN) {
            errs() << VN << "::XcodeMlTraverseDecl\n";
    }
    return static_cast<RAV*>(this)->TraverseDecl(D);
}

#define ABSTRACT_DECL(DECL)
#define DECL(CLASS, BASE)                                            \
    bool XcodeMlRAV::Traverse##CLASS##Decl(CLASS##Decl *D) {         \
        const char *VN = otherside->getVisitorName();                \
        if (OptTraceRAV && VN) {                                     \
            errs() << VN << "::       Traverse" #CLASS "Decl\n";     \
        }                                                            \
        return otherside->XcodeMlTraverse##CLASS##Decl(D);           \
    }                                                                \
    bool XcodeMlRAV::XcodeMlTraverse##CLASS##Decl(CLASS##Decl *D) {  \
        const char *VN = otherside->getVisitorName();                \
        if (OptTraceRAV && VN) {                                     \
            errs() << VN << "::XcodeMlTraverse" #CLASS "Decl\n";     \
        }                                                            \
        return static_cast<RAV*>(this)->Traverse##CLASS##Decl(D);    \
    }
#include "clang/AST/DeclNodes.inc"

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
