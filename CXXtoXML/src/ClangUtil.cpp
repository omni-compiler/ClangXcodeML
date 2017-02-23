#include <string>

#include "clang/Basic/Linkage.h"

#include "ClangUtil.h"

using namespace clang;

const char* stringifyLanguageLinkage(LanguageLinkage ll) {
  switch (ll) {
    case CLanguageLinkage:
      return "C";
    case CXXLanguageLinkage:
      return "C++";
    case NoLanguageLinkage:
      return "no";
  }
}

