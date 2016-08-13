#include <cassert>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "SymbolAnalyzer.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"

namespace XcodeMl {
  TypeRef get(const Environment& env, const DataTypeIdent& type) {
    auto search = env.typemap.find(type);
    if (search != env.end()) {
      return search->second;
    } else {
      return nullptr;
    }
  }
}
