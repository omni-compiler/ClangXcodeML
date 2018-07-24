#ifndef CLANGCLASSHANDLER_H
#define CLANGCLASSHANDLER_H

using ClangStmtHandlerType =
    AttrProc<XcodeMl::CodeFragment, const CodeBuilder &, SourceInfo &>;

extern const ClangStmtHandlerType ClangStmtHandler;

#endif /* !CLANGCLASSHANDLER_H */
