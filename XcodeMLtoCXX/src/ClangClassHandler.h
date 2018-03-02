#ifndef CLANGCLASSHANDLER_H
#define CLANGCLASSHANDLER_H

using ClangClassHandler =
    AttrProc<XcodeMl::CodeFragment, const CodeBuilder &, SourceInfo &>;

using ConstClangClassHandler =
    AttrProc<XcodeMl::CodeFragment, const SourceInfo &>;

extern const ClangClassHandler ClangStmtHandler;
extern const ClangClassHandler ClangDeclHandler;
extern const ClangClassHandler ClassDefinitionDeclHandler;
extern const ConstClangClassHandler ClangNestedNameSpecHandler;

#endif /* !CLANGCLASSHANDLER_H */
