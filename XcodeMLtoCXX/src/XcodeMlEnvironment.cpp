#include <functional>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cassert>
#include "StringTree.h"
#include "Symbol.h"
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

  const TypeRef& Environment::at(const std::string& dataTypeIdent) const {
    return map.at(dataTypeIdent);
  }

  TypeRef& Environment::at(const std::string& dataTypeIdent) {
    return map.at(dataTypeIdent);
  }

  const TypeRef& Environment::getReturnType(
    const std::string& dataTypeIdent
  ) const {
    try {
      return returnMap.at(dataTypeIdent);
    } catch (const std::out_of_range& e) {
      const auto msg =
        std::string("return type of '")
        + dataTypeIdent
        + "' not found in XcodeMl::Environment";
      throw std::out_of_range(msg);
    }
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

  TypeRef& Environment::at_or_throw(
      Environment::TypeMap& map,
      const std::string& key,
      const std::string& name
  ) const {
    try {
      return map.at(key);
    } catch (const std::out_of_range& e) {
      const auto msg =
        name + " '" + key + "' not found in XcodeMl::Environment";
      throw std::out_of_range(msg);
    }
  }

  const TypeRef& Environment::at_or_throw(
      const Environment::TypeMap& map,
      const std::string& key,
      const std::string& name
  ) const {
    try {
      return map.at(key);
    } catch (const std::out_of_range& e) {
      const auto msg =
        name + " '" + key + "' not found in XcodeMl::Environment";
      throw std::out_of_range(msg);
    }
  }

}
