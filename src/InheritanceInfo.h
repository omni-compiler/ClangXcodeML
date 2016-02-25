#ifndef INHERITANCEINFO_H
#define INHERITANCEINFO_H

#include "Hash.h"
#include <vector>

class BaseClass {
  clang::QualType baseType;
  clang::AccessSpecifier accessSpec;
  bool virtuality;
public:
  BaseClass(clang::QualType, clang::AccessSpecifier, bool);
  clang::QualType type();
  clang::AccessSpecifier access();
  bool isVirtual();
};

class InheritanceInfo {
  std::unordered_map<clang::QualType, std::vector<BaseClass> > inheritance;
public:
  InheritanceInfo() = default;
  InheritanceInfo(const InheritanceInfo&) = delete;
  InheritanceInfo(InheritanceInfo&&) = delete;
  InheritanceInfo& operator=(const InheritanceInfo &) = delete;
  InheritanceInfo& operator=(const InheritanceInfo &&) = delete;
  std::vector<BaseClass> getInheritance(clang::QualType type);
  void addInheritance(clang::QualType derived, BaseClass base);
};

#endif /* !INHERITANCEINFO_H */
