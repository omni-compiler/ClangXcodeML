#ifndef CXXCODEGEN_H
#define CXXCODEGEN_H

namespace CXXCodeGen {

struct space_t {};

extern const space_t space;

struct newline_t {};

extern const newline_t newline;

struct StreamImpl;

class Stream {
public:
  Stream();
  ~Stream();
  Stream(Stream &&);
  Stream &operator=(Stream &&);
  std::string str();
  void indent(size_t);
  void insertNewLine();
  void insertSpace();
  void insert(const std::string &);
  void unindent(size_t);

private:
  std::unique_ptr<StreamImpl> pimpl;
};

} // namespace CXXCodeGen

CXXCodeGen::Stream &operator<<(
    CXXCodeGen::Stream &, const CXXCodeGen::space_t &);
CXXCodeGen::Stream &operator<<(
    CXXCodeGen::Stream &, const CXXCodeGen::newline_t &);
CXXCodeGen::Stream &operator<<(CXXCodeGen::Stream &, const std::string &);
CXXCodeGen::Stream &operator<<(CXXCodeGen::Stream &, char);

#endif /* !CXXCODEGEN_H */
