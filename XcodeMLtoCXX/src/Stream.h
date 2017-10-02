#ifndef CXXCODEGEN_H
#define CXXCODEGEN_H

namespace CXXCodeGen {

struct space_t {};

extern const space_t space;

struct newline_t {};

extern const newline_t newline;

class Stream {
public:
  Stream();
  std::string str();
  void indent(size_t);
  void unindent(size_t);
  Stream &operator<<(const space_t &);
  Stream &operator<<(const newline_t &);
  Stream &operator<<(const std::string &);
  Stream &operator<<(char);

private:
  void outputIndentation();
  void emit(const std::string &);

  std::stringstream ss;
  size_t curIndent;
  bool alreadyIndented;
  char lastChar;
};
}

#endif /* !CXXCODEGEN_H */
