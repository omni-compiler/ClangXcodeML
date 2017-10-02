#include "XcodeMlVisitorBase.h"
#include "clang/Driver/Options.h"
#include "clang/Lex/Lexer.h"

#include <type_traits>
#include <unistd.h>

using namespace clang;
using namespace llvm;

static cl::opt<bool> OptEmitSourceFileName(
    "file", cl::desc("emit 'file'"), cl::cat(C2XcodeMLCategory));
static cl::opt<bool> OptEmitSourceLineNo(
    "lineno", cl::desc("emit 'lineno'"), cl::cat(C2XcodeMLCategory));
static cl::opt<bool> OptEmitSourceColumn(
    "column", cl::desc("emit 'column'"), cl::cat(C2XcodeMLCategory));
static cl::opt<bool> OptEmitSourceRange(
    "range", cl::desc("emit 'range'"), cl::cat(C2XcodeMLCategory));

// implementation of XcodeMlVisitorBaseImpl

XcodeMlVisitorBaseImpl::XcodeMlVisitorBaseImpl(
    MangleContext *MC, xmlNodePtr CurNode, TypeTableInfo *TTI)
    : XcodeMlRAVpool(this),
      mangleContext(MC),
      curNode(CurNode),
      typetableinfo(TTI){};

xmlNodePtr
XcodeMlVisitorBaseImpl::addChild(const char *Name, const char *Content) {
  return xmlNewTextChild(curNode, nullptr, BAD_CAST Name, BAD_CAST Content);
}

xmlNodePtr
XcodeMlVisitorBaseImpl::addChild(const char *Name, xmlNodePtr N) {
  return xmlNewTextChild(N, nullptr, BAD_CAST Name, nullptr);
}

xmlNodePtr
XcodeMlVisitorBaseImpl::addName(const char *FullName, const char *Content) {
  xmlNodePtr node = addChild("name", Content);
  newProp("fullName", FullName, node);
  return node;
}

xmlNodePtr
XcodeMlVisitorBaseImpl::addName(clang::NamedDecl *ND, const char *Content) {
  return addName(ND->getQualifiedNameAsString().c_str(), Content);
}

void
XcodeMlVisitorBaseImpl::newChild(const char *Name, const char *Content) {
  curNode = xmlNewTextChild(curNode, nullptr, BAD_CAST Name, BAD_CAST Content);
}

void
XcodeMlVisitorBaseImpl::newName(const char *FullName, const char *Content) {
  curNode = addName(FullName, Content);
}

void
XcodeMlVisitorBaseImpl::newName(clang::NamedDecl *ND, const char *Content) {
  curNode = addName(ND, Content);
}

void
XcodeMlVisitorBaseImpl::newProp(const char *Name, int Val, xmlNodePtr N) {
  if (!N)
    N = curNode;
  xmlChar Buf[BUFSIZ];
  xmlStrPrintf(Buf, BUFSIZ, BAD_CAST "%d", Val);
  xmlNewProp(N, BAD_CAST Name, Buf);
}

void
XcodeMlVisitorBaseImpl::newProp(
    const char *Name, const char *Val, xmlNodePtr N) {
  if (!N)
    N = curNode;
  xmlNewProp(N, BAD_CAST Name, BAD_CAST Val);
}

void
XcodeMlVisitorBaseImpl::newComment(const xmlChar *str, xmlNodePtr N) {
  if (!N)
    N = curNode;
  xmlChar Buf[BUFSIZ];
  const char *VN = getVisitorName();
  if (VN) {
    xmlStrPrintf(Buf, BUFSIZ, BAD_CAST "%s::%s", BAD_CAST VN, str);
    xmlNodePtr Comment = xmlNewComment(Buf);
    xmlAddChild(N, Comment);
    // errs() << (const char *)Buf << "\n";
  }
}

void
XcodeMlVisitorBaseImpl::newComment(const char *str, xmlNodePtr N) {
  newComment(BAD_CAST str, N);
}

void
XcodeMlVisitorBaseImpl::newComment(const std::string &str, xmlNodePtr N) {
  newComment(str.c_str(), N);
}

void
XcodeMlVisitorBaseImpl::setLocation(SourceLocation Loc, xmlNodePtr N) {
  if (!N)
    N = curNode;
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
XcodeMlVisitorBaseImpl::contentBySource(
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

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 4
/// End:
///
