#ifndef CLANGOPERATOR_H
#define CLANGOPERATOR_H

const char*
BOtoElemName(clang::BinaryOperatorKind);

const char*
UOtoElemName(clang::UnaryOperatorKind);

const char*
OverloadedOperatorKindToString(clang::OverloadedOperatorKind, unsigned);

#endif /* !CLANGOPERATOR_H */
