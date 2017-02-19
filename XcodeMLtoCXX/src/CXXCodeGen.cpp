#include <cassert>
#include <sstream>
#include <string>

#include "CXXCodeGen.h"

namespace CXXCodeGen {

const space_t space = {};

const newline_t newline = {};

std::string Stream::str() {
  return  ss.str();
}

void Stream::indent(size_t amount) {
  curIndent += amount;
}

void Stream::unindent(size_t amount) {
  assert(curIndent >= amount);
  curIndent -= amount;
}

Stream& Stream::operator <<(const space_t&) {
  const std::string separaters = "\n\t ";
  if (separaters.find(lastChar) != std::string::npos) {
    ss << " ";
    lastChar = " ";
  }
  return *this;
}

Stream& Stream::operator <<(const newline_t&) {
  ss << "\n";
  lastChar = "\n";
  alreadyIndented = false;
  return *this;
}

void Stream::outputIndentation() {
  if (alreadyIndented) {
    return;
  }

  for (size_t i = 0; i < curIndent; ++i) {
    ss << "\t";
  }
  lastChar = '\t';
  alreadyIndented = true;
}

void Stream::emit(const std::string& str) {
  if (str.empty()) {
    return;
  }
  ss << str;
  lastChar = str.back();
}

}
