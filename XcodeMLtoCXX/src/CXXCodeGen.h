#ifndef CXXCODEGEN_H
#define CXXCODEGEN_H

namespace CXXCodeGen {

struct space_t {
};

extern const space_t space;

struct newline_t {
};

extern const newline_t newline;

class Stream {
public:
  Stream() = default;
  std::string str();
  Stream& operator <<(const space_t&);
  Stream& operator <<(const newline_t&);
  Stream& operator <<(const std::string&);

private:
  std::stringstream ss;
  size_t curIndent;
  char lastChar;
};

}

#endif /* !CXXCODEGEN_H */
