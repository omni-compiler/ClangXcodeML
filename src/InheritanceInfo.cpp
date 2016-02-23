#include "InheritanceInfo.h"

std::vector<clang::QualType> InheritanceInfo::getInheritance(clang::QualType type) {
  return inheritance[type];
}

void InheritanceInfo::addInheritance(clang::QualType derived, clang::QualType base) {
  inheritance[derived].push_back(base);
}
