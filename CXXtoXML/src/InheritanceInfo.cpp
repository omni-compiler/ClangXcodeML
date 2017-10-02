#include "InheritanceInfo.h"

AccessSpec::AccessSpec(clang::AccessSpecifier AS) : accessSpec(AS) {
}

AccessSpec::operator clang::AccessSpecifier() const {
  return accessSpec;
}

std::string
AccessSpec::to_string() const {
  return c_str();
}

const char *
AccessSpec::c_str() const {
  switch (accessSpec) {
  case clang::AS_public: return "public";
  case clang::AS_private: return "private";
  case clang::AS_protected: return "protected";
  case clang::AS_none: return "none";
  }
}

BaseClass::BaseClass(clang::QualType t, clang::AccessSpecifier a, bool v)
    : baseType(t), accessSpec(a), virtuality(v) {
}

clang::QualType
BaseClass::type() {
  return baseType;
}

AccessSpec
BaseClass::access() {
  return accessSpec;
}

bool
BaseClass::isVirtual() {
  return virtuality;
}

std::vector<BaseClass>
InheritanceInfo::getInheritance(clang::QualType type) {
  return inheritance[type];
}

void
InheritanceInfo::addInheritance(clang::QualType derived, BaseClass base) {
  inheritance[derived].push_back(base);
}
