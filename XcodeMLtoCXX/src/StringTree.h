#ifndef STRINGTREE_H
#define STRINGTREE_H

namespace CXXCodeGen {

enum class StringTreeKind {
  /*! inner node (a node that is not a leaf) */
  Inner,
  /*! leaf node containing a string */
  Token,
  /*! leaf node which represents a "\n" */
  NewLine,
};

class InnerNode;

class Stream;

class StringTree {
public:
  explicit StringTree(StringTreeKind);
  virtual ~StringTree() = 0;
  virtual StringTree *clone() const = 0;
  virtual void flush(Stream &) const = 0;
  virtual InnerNode *lift() const = 0;
  StringTreeKind getKind() const;

protected:
  StringTree(const StringTree &) = default;

private:
  const StringTreeKind kind;
};

using StringTreeRef = std::shared_ptr<StringTree>;

class InnerNode : public StringTree {
public:
  static bool classof(const StringTree *);
  explicit InnerNode(const std::vector<StringTreeRef> &);
  ~InnerNode() = default;
  StringTree *clone() const override;
  void flush(Stream &) const override;
  InnerNode *lift() const override;
  void amend(const StringTreeRef &);

protected:
  InnerNode(const InnerNode &) = default;

private:
  std::vector<StringTreeRef> children;
};

class TokenNode : public StringTree {
public:
  static bool classof(const StringTree *);
  explicit TokenNode(const std::string &);
  ~TokenNode() = default;
  StringTree *clone() const override;
  void flush(Stream &) const override;
  InnerNode *lift() const override;

protected:
  TokenNode(const TokenNode &) = default;

private:
  std::string token;
};

class NewLineNode : public StringTree {
public:
  static bool classof(const StringTree *);
  ~NewLineNode() = default;
  StringTree *clone() const override;
  void flush(Stream &) const override;
  InnerNode *lift() const override;

  static StringTreeRef getsingleton();

protected:
  explicit NewLineNode();
  NewLineNode(const NewLineNode &) = default;
};

extern const NewLineNode newlinenode;

class LineInfoNode : public StringTree {
public:
  static bool classof(const StringTree *);
  explicit LineInfoNode(const std::string &filename, size_t lineno);
  ~LineInfoNode() = default;
  StringTree *clone() const override;
  void flush(Stream &) const override;
  InnerNode *lift() const override;

protected:
  LineInfoNode(const LineInfoNode &) = default;

private:
  std::string filename;
  size_t lineno;
};

std::string to_string(const StringTreeRef &);

StringTreeRef makeVoidNode();
StringTreeRef makeNewLineNode();
StringTreeRef makeInnerNode(const std::vector<StringTreeRef> &);
StringTreeRef makeTokenNode(const std::string &);

StringTreeRef insertNewLines(const std::vector<StringTreeRef> &);
StringTreeRef separateByBlankLines(const std::vector<StringTreeRef> &);
StringTreeRef join(const std::string &, const std::vector<StringTreeRef> &);
StringTreeRef wrapWithParen(const StringTreeRef &);
StringTreeRef wrapWithSquareBracket(const StringTreeRef &);
StringTreeRef wrapWithBrace(const StringTreeRef &);
}

CXXCodeGen::StringTreeRef operator+(
    const CXXCodeGen::StringTreeRef &, const CXXCodeGen::StringTreeRef &);

#endif /* !STRINGTREE_H */
