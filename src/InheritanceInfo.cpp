#include "InheritanceInfo.h"

BaseClass::BaseClass(clang::QualType t, clang::AccessSpecifier a, bool v):
  baseType(t),
  accessSpec(a),
  virtuality(v)
{}

clang::QualType BaseClass::type() {
  return baseType;
}

clang::AccessSpecifier BaseClass::access() {
  return accessSpec;
}

bool BaseClass::isVirtual() {
  return virtuality;
}

std::vector<BaseClass> InheritanceInfo::getInheritance(clang::QualType type) {
  return inheritance[type];
}

void InheritanceInfo::addInheritance(clang::QualType derived, BaseClass base) {
  inheritance[derived].push_back(base);
}
