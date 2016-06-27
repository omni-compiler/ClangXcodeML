#ifndef HASH_H
#define HASH_H

#include "clang/AST/Type.h"
#include <unordered_map>

namespace std {
    template<>
    struct hash<clang::QualType> {
        size_t operator()(const clang::QualType T) const {
            union union_for_hash {
                size_t value;
                clang::QualType originalT;
                union_for_hash(clang::QualType T) : originalT(T) {};
            };
            const union_for_hash tmp(T);
            return tmp.value;
        }
    };
}

#endif /* !HASH_H */
