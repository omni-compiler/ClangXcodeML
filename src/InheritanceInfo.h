#ifndef INHERITANCEINFO_H
#define INHERITANCEINFO_H

#include "Hash.h"
#include <vector>

class InheritanceInfo {
  std::unordered_map<clang::QualType, std::vector<clang::QualType> > inheritance;
public:
  InheritanceInfo() = default;
  InheritanceInfo(const InheritanceInfo&) = delete;
  InheritanceInfo(InheritanceInfo&&) = delete;
  InheritanceInfo& operator=(const InheritanceInfo &) = delete;
  InheritanceInfo& operator=(const InheritanceInfo &&) = delete;
  std::vector<clang::QualType> getInheritance(clang::QualType type);
  void addInheritance(clang::QualType derived, clang::QualType base);
};

#endif /* !INHERITANCEINFO_H */
