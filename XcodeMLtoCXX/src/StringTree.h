#ifndef STRINGTREE_H
#define STRINGTREE_H

namespace CXXCodeGen {

enum class StringTreeKind {
  /*! inner node (a node that is not a leaf) */
  Inner,
  /*! leaf node containing a string */
  Token,
};

class InnerNode;

class StringTree {
public:
  explicit StringTree(StringTreeKind);
  virtual ~StringTree() = 0;
  virtual StringTree* clone() const = 0;
  virtual void flush(std::stringstream&) const = 0;
  virtual InnerNode* lift() const = 0;
  StringTreeKind getKind() const;
protected:
  StringTree(const StringTree&) = default;
private:
  const StringTreeKind kind;
};

using StringTreeRef = std::shared_ptr<StringTree>;

class InnerNode : public StringTree {
public:
  static bool classof(const StringTree*);
  explicit InnerNode(const std::vector<StringTreeRef>&);
  ~InnerNode() = default;
  StringTree* clone() const override;
  void flush(std::stringstream&) const override;
  InnerNode* lift() const override;
  void amend(const StringTreeRef&);
protected:
  InnerNode(const InnerNode&) = default;
private:
  std::vector<StringTreeRef> children;
};

class TokenNode : public StringTree {
public:
  static bool classof(const StringTree*);
  explicit TokenNode(const std::string&);
  ~TokenNode() = default;
  StringTree* clone() const override;
  void flush(std::stringstream&) const override;
  InnerNode* lift() const override;
protected:
  TokenNode(const TokenNode&) = default;
private:
  std::string token;
};

StringTreeRef makeVoidNode();
StringTreeRef makeNewLineNode();
StringTreeRef makeInnerNode(const std::vector<StringTreeRef>&);
StringTreeRef makeTokenNode(const std::string&);

}

CXXCodeGen::StringTreeRef operator+(
    const CXXCodeGen::StringTreeRef&,
    const CXXCodeGen::StringTreeRef&);

#endif /* !STRINGTREE_H */

