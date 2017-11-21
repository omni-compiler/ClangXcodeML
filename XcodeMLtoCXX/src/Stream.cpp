#include <cassert>
#include <memory>
#include <sstream>
#include <string>

#include "Stream.h"

namespace {

template <typename T, typename... Ts>
std::unique_ptr<T>
make_unique(Ts &&... params) {
  return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

} // namespace

namespace CXXCodeGen {

const space_t space = {};

const newline_t newline = {};

Stream::Stream() : ss(), curIndent(0), alreadyIndented(false), lastChar('\n') {
}

std::string
Stream::str() {
  return ss.str();
}

void
Stream::indent(size_t amount) {
  curIndent += amount;
}

void
Stream::unindent(size_t amount) {
  assert(curIndent >= amount);
  curIndent -= amount;
}

void
Stream::insertSpace() {
  const std::string separators = "\n\t ";
  if (separators.find(lastChar) == std::string::npos) {
    emit(" ");
  }
}

void
Stream::insertNewLine() {
  emit("\n");
  alreadyIndented = false;
}

namespace {

bool
isAllowedInIdent(char c) {
  /* FIXME: C++ allows universal character */
  return isalnum(c) || c == '_';
}

bool
shouldInterleaveSpace(char last, char next) {
  const std::string operators = "+-*/%^&|!><";
  const std::string repeatables = "+-><&|=";
  return (isAllowedInIdent(last) && isAllowedInIdent(next))
      || (operators.find(last) != std::string::npos && next == '=')
      || (last == next && repeatables.find(last) != std::string::npos)
      || (last == '-' && next == '>') || // `->`
      (last == '>' && next == '*'); // `->*`
}

} // namespace

void
Stream::insert(const std::string &token) {
  if (token.empty()) {
    return;
  }

  outputIndentation();

  if (shouldInterleaveSpace(lastChar, token[0])) {
    emit(" ");
  }
  emit(token);
}

void
Stream::outputIndentation() {
  if (alreadyIndented) {
    return;
  }

  for (size_t i = 0; i < curIndent; ++i) {
    ss << "\t";
  }
  lastChar = '\t';
  alreadyIndented = true;
}

void
Stream::emit(const std::string &str) {
  if (str.empty()) {
    return;
  }
  ss << str;
  lastChar = str.back();
}

} // namespace CXXCodeGen

CXXCodeGen::Stream &operator<<(
    CXXCodeGen::Stream &s, const CXXCodeGen::space_t &) {
  s.insertSpace();
  return s;
}

CXXCodeGen::Stream &operator<<(
    CXXCodeGen::Stream &s, const CXXCodeGen::newline_t &) {
  s.insertNewLine();
  return s;
}

CXXCodeGen::Stream &operator<<(CXXCodeGen::Stream &s, const std::string &str) {
  s.insert(str);
  return s;
}

CXXCodeGen::Stream &operator<<(CXXCodeGen::Stream &s, char c) {
  s.insert(std::string(1, c));
  return s;
}
