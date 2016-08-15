#include <functional>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cassert>
#include "SymbolAnalyzer.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"

namespace XcodeMl {

  const TypeRef& Environment::operator[](
    const std::string& dataTypeIdent
  ) const {
    return map.at(dataTypeIdent);
  }

  TypeRef& Environment::operator[](
    const std::string& dataTypeIdent
  ) {
    if (map.find(dataTypeIdent) == map.end()) {
      keys.push_back(dataTypeIdent);
    }
    return map[dataTypeIdent];
  }

  const TypeRef& Environment::getReturnType(
    const std::string& dataTypeIdent
  ) const {
    return returnMap.at(dataTypeIdent);
  }

  void Environment::setReturnType(
    const std::string& dataTypeIdent,
    const TypeRef& type
  ) {
    returnMap[dataTypeIdent] = type;
  }

  const std::vector<std::string>& Environment::getKeys(void) const {
    return keys;
  }

}
