#ifndef INHERITANCEINFO_H
#define INHERITANCEINFO_H

#include "Hash.h"
#include <vector>
#include <string>

class AccessSpec {
  clang::AccessSpecifier accessSpec;

public:
  AccessSpec(clang::AccessSpecifier);
  operator clang::AccessSpecifier() const;
  std::string to_string() const;
  const char *c_str() const;
};

class BaseClass {
  clang::QualType baseType;
  AccessSpec accessSpec;
  bool virtuality;

public:
  BaseClass(clang::QualType, clang::AccessSpecifier, bool);
  clang::QualType type();
  AccessSpec access();
  bool isVirtual();
};

class InheritanceInfo {
  std::unordered_map<clang::QualType, std::vector<BaseClass>> inheritance;

public:
  InheritanceInfo() = default;
  InheritanceInfo(const InheritanceInfo &) = delete;
  InheritanceInfo(InheritanceInfo &&) = delete;
  InheritanceInfo &operator=(const InheritanceInfo &) = delete;
  InheritanceInfo &operator=(const InheritanceInfo &&) = delete;
  std::vector<BaseClass> getInheritance(clang::QualType type);
  void addInheritance(clang::QualType derived, BaseClass base);
};

#endif /* !INHERITANCEINFO_H */
