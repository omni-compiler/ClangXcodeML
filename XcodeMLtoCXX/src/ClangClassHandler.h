#ifndef CLANGCLASSHANDLER_H
#define CLANGCLASSHANDLER_H

using ClangClassHandler =
    AttrProc<XcodeMl::CodeFragment, const CodeBuilder &, SourceInfo &>;

extern const ClangClassHandler ClangStmtHandler;
extern const ClangClassHandler ClangDeclHandler;
extern const ClangClassHandler ClassDefinitionDeclHandler;
extern const ClangClassHandler ClangTypeLocHandler;
extern const ClangClassHandler ClangNestedNameSpecHandler;

#endif /* !CLANGCLASSHANDLER_H */
