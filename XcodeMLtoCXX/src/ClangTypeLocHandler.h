#ifndef CLANGTYPELOCHANDLER_H
#define CLANGTYPELOCHANDLER_H

using ClangTypeLocHandlerType =
    AttrProc<XcodeMl::CodeFragment, const CodeBuilder &, SourceInfo &>;

extern const ClangTypeLocHandlerType ClangTypeLocHandler;

#endif /* !CLANGTYPELOCHANDLER_H */
