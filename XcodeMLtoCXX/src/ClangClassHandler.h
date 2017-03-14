#ifndef CLANGCLASSHANDLER_H
#define CLANGCLASSHANDLER_H

using ClangClassHandler = AttrProc<
  void,
  const CodeBuilder&,
  SourceInfo&,
  CXXCodeGen::Stream&>;

extern const ClangClassHandler ClangStmtHandler;

#endif /* !CLANGCLASSHANDLER_H */
