#ifndef CLANGCLASSHANDLER_H
#define CLANGCLASSHANDLER_H

using ClangClassHandler = AttrProc<
  XcodeMl::CodeFragment,
  const CodeBuilder&,
  SourceInfo&>;

extern const ClangClassHandler ClangStmtHandler;

#endif /* !CLANGCLASSHANDLER_H */
