#ifndef CLANGOPERATOR_H
#define CLANGOPERATOR_H

const char*
BOtoElemName(clang::BinaryOperatorKind);

const char*
UOtoElemName(clang::UnaryOperatorKind);

const char*
OverloadedOperatorKindToString(clang::OverloadedOperatorKind, unsigned);

namespace clang {
  class FunctionDecl;
}
const char*
getOperatorString(const clang::FunctionDecl*);

#endif /* !CLANGOPERATOR_H */
