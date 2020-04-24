#include <string>

#include "clang/Basic/Linkage.h"

#include "ClangUtil.h"

using namespace clang;

const char *
stringifyLanguageLinkage(LanguageLinkage ll) {
  switch (ll) {
  case CLanguageLinkage: return "C";
  case CXXLanguageLinkage: return "C++";
  case NoLanguageLinkage: return "no";
  }
}

const char *
stringifyLinkage(Linkage l) {
  switch (l) {
  case NoLinkage: return "NoLinkage";
  case InternalLinkage: return "InternalLinkage";
  case ExternalLinkage: return "ExternalLinkage";

  case UniqueExternalLinkage:
    // external linkage but within unnamed namespace
    return "UniqueExternalLinkage";
  case VisibleNoLinkage: return "VisibleNoLinkage";
  case ModuleLinkage: return "ModuleLinkage";
  default:abort();
  }

  return "Unknown";
}
