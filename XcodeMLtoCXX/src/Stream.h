#ifndef CXXCODEGEN_H
#define CXXCODEGEN_H

namespace CXXCodeGen {

/*! \brief Represents space character(' '). */
struct space_t {};

extern const space_t space;

/*! \brief Represents newline character('\n'). */
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
  /*! \brief Increases indent. */
  void indent(size_t);
  void insertNewLine();
  /*! \brief Emits a space character if necessary. */
  void insertSpace();
  /*! \brief Emits the given string.
   *
   * It emits a space character (token separator) if necessary.
   */
  void insert(const std::string &);
  /*! \brief Decreases indent. */
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
