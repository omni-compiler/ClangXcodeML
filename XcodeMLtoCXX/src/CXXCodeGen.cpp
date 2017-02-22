#include <cassert>
#include <sstream>
#include <string>

#include "CXXCodeGen.h"

namespace CXXCodeGen {

const space_t space = {};

const newline_t newline = {};

Stream::Stream():
  ss(),
  curIndent(0),
  alreadyIndented(false),
  lastChar('\n')
{}

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
  const std::string separators = "\n\t ";
  if (separators.find(lastChar) == std::string::npos) {
    emit(" ");
  }
  return *this;
}

Stream& Stream::operator <<(const newline_t&) {
  emit("\n");
  alreadyIndented = false;
  return *this;
}

static bool isAllowedInIdent(char c) {
  /* FIXME: C++ allows universal character */
  return isalnum(c) || c == '_';
}

static bool shouldInterleaveSpace(char last, char next) {
  const std::string operators = "+-*/%^&|!><";
  const std::string repeatables = "+-><&|=";
  return
    (isAllowedInIdent(last) && isAllowedInIdent(next)) ||
    (operators.find(last) != std::string::npos && next == '=') ||
    (last == next && repeatables.find(last) != std::string::npos) ||
    (last == '-' && next == '>') || // `->`
    (last == '>' && next == '*'); // `->*`
}

Stream& Stream::operator <<(const std::string& token) {
  if (token.empty()) {
    return *this;
  }

  outputIndentation();

  if (shouldInterleaveSpace(lastChar, token[0])) {
    emit(" ");
  }
  emit(token);

  return *this;
}

Stream& Stream::operator <<(char c) {
  return (*this) << std::string(1, c);
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
