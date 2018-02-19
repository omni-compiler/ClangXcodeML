#ifndef CLANGCLASSHANDLER_H
#define CLANGCLASSHANDLER_H

using ClangClassHandler =
    AttrProc<XcodeMl::CodeFragment, const CodeBuilder &, SourceInfo &>;

extern const ClangClassHandler ClangStmtHandler;
extern const ClangClassHandler ClangDeclHandler;
extern const ClangClassHandler ClassDefinitionDeclHandler;
extern const ClangClassHandler ClangTypeLocHandler;

#endif /* !CLANGCLASSHANDLER_H */
