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
  void insertNewLine();
  void insertSpace();
  void insert(const std::string &);
  void unindent(size_t);

private:
  void outputIndentation();
  void emit(const std::string &);

  std::stringstream ss;
  size_t curIndent;
  bool alreadyIndented;
  char lastChar;
};

} // namespace CXXCodeGen

#endif /* !CXXCODEGEN_H */
