#include <functional>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <cassert>
#include <iostream>
#include <libxml/tree.h>
#include "llvm/ADT/Optional.h"
#include "StringTree.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlTypeTable.h"

namespace XcodeMl {

const TypeRef &TypeTable::operator[](const std::string &dataTypeIdent) const {
  return at_or_throw(map, dataTypeIdent, "Data type");
}

TypeRef &TypeTable::operator[](const std::string &dataTypeIdent) {
  if (map.find(dataTypeIdent) == map.end()) {
    keys.push_back(dataTypeIdent);
  }
  return map[dataTypeIdent];
}

const TypeRef &
TypeTable::at(const std::string &dataTypeIdent) const {
  return at_or_throw(map, dataTypeIdent, "Data type");
}

TypeRef &
TypeTable::at(const std::string &dataTypeIdent) {
  return at_or_throw(map, dataTypeIdent, "Data type");
}

const TypeRef &
TypeTable::getReturnType(const std::string &dataTypeIdent) const {
  return at_or_throw(returnMap, dataTypeIdent, "Return type of");
}

void
TypeTable::setReturnType(
    const std::string &dataTypeIdent, const TypeRef &type) {
  returnMap[dataTypeIdent] = type;
}

bool
TypeTable::exists(const std::string &dataTypeIdent) const {
  return map.find(dataTypeIdent) != map.end();
}

const std::vector<std::string> &
TypeTable::getKeys(void) const {
  return keys;
}

TypeRef &
TypeTable::at_or_throw(TypeTable::TypeMap &map,
    const std::string &key,
    const std::string &name) const {
  try {
    return map.at(key);
  } catch (const std::out_of_range &e) {
    const auto msg = name + " '" + key + "' not found in XcodeMl::TypeTable";
    throw std::out_of_range(msg);
  }
}
void TypeTable::dump()
{
  for(const auto &e : map){

  }
}
const TypeRef &
TypeTable::at_or_throw(const TypeTable::TypeMap &map,
    const std::string &key,
    const std::string &name) const {
  try {
    return map.at(key);
  } catch (const std::out_of_range &e) {
    const auto msg = name + " '" + key + "' not found in XcodeMl::TypeTable";
    throw std::out_of_range(msg);
  }
}
}
