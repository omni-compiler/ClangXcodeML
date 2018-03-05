#ifndef CLANGDECLHANDLER_H
#define CLANGDECLHANDLER_H

using ClangDeclHandlerType =
    AttrProc<XcodeMl::CodeFragment, const CodeBuilder &, SourceInfo &>;

extern const ClangDeclHandlerType ClangDeclHandler;
extern const ClangDeclHandlerType ClangDeclHandlerInClass;

#endif /* !CLANGDECLHANDLER_H */
