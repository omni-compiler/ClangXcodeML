#include <cassert>
#include <sstream>
#include <string>

#include "CXXCodeGen.h"

namespace CXXCodeGen {

const space_t space = {};

const newline_t newline = {};

const indent_t indent;

const indent_t unindent;

std::string Stream::str() {
  return  ss.str();
}

}
