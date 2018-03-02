#ifndef CLANGNAMESPECHANDLER_H
#define CLANGNAMESPECHANDLER_H

using ClangNestedNameSpecHandlerType =
    AttrProc<XcodeMl::CodeFragment, const SourceInfo &>;

extern const ClangNestedNameSpecHandlerType ClangNestedNameSpecHandler;

#endif /* !CLANGNAMESPECHANDLER_H */
