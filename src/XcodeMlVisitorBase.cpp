#include "XcodeMlVisitorBase.h"
#include "clang/Driver/Options.h"
#include "clang/Lex/Lexer.h"

#include <type_traits>

using namespace clang;
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

// implementation of XcodeMlVisitorBaseImpl

XcodeMlVisitorBaseImpl::XcodeMlVisitorBaseImpl(MangleContext *MC,
                                               xmlNodePtr Parent,
                                               xmlNodePtr CurNode,
                                               TypeTableInfo *TTI)
    : XcodeMlRAVpool(this),
      mangleContext(MC), parentNode(Parent), curNode(CurNode),
      typetableinfo(TTI), contentString("") {};

void XcodeMlVisitorBaseImpl::addChild(const char *Name, const char *Content) {
    if (!Content && contentString.length() > 0) {
        Content = contentString.c_str();
    }
    xmlNewChild(curNode, nullptr, BAD_CAST Name, BAD_CAST Content);
}

void XcodeMlVisitorBaseImpl::newChild(const char *Name, const char *Content) {
    if (!Content && contentString.length() > 0) {
        Content = contentString.c_str();
    }
    curNode = xmlNewChild(curNode, nullptr, BAD_CAST Name, BAD_CAST Content);
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

void XcodeMlVisitorBaseImpl::newComment(const xmlChar *str, xmlNodePtr PN) {
    if (!PN) PN = parentNode;
    xmlChar Buf[BUFSIZ];
    const char *VN = getVisitorName();
    if (VN) {
        xmlStrPrintf(Buf, BUFSIZ,
                     BAD_CAST "%s::%s", BAD_CAST VN, str);
        xmlNodePtr Comment = xmlNewComment(Buf);
        xmlAddChild(PN, Comment);
        //errs() << (const char *)Buf << "\n";
    }
}

void XcodeMlVisitorBaseImpl::newComment(const char *str, xmlNodePtr PN) {
    newComment(BAD_CAST str, PN);
}

void XcodeMlVisitorBaseImpl::setLocation(SourceLocation Loc, xmlNodePtr N) {
    if (!N) N = curNode;
    FullSourceLoc FLoc = mangleContext->getASTContext().getFullLoc(Loc);
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

void XcodeMlVisitorBaseImpl::setContentBySource(SourceLocation LocStart,
                                                SourceLocation LocEnd) {
    ASTContext &CXT = mangleContext->getASTContext();
    SourceManager &SM = CXT.getSourceManager();
    SourceLocation LocEndOfToken = Lexer::getLocForEndOfToken(LocEnd, 0, SM,
                                                              CXT.getLangOpts());
    const char *b = SM.getCharacterData(LocStart);
    const char *e = SM.getCharacterData(LocEndOfToken);
    contentString = std::string(b, e - b);
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
