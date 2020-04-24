#include <memory>
#include <vector>
#include <string>
#include "llvm/Support/Casting.h"

#include "Stream.h"
#include "StringTree.h"

namespace CXXCodeGen {

StringTree::StringTree(StringTreeKind k) : kind(k) {
}

StringTree::~StringTree() {
}

StringTreeKind
StringTree::getKind() const {
  return kind;
}

bool
InnerNode::classof(const StringTree *node) {
  return node->getKind() == StringTreeKind::Inner;
}

InnerNode::InnerNode(const std::vector<StringTreeRef> &v)
    : StringTree(StringTreeKind::Inner), children(v) {
}

StringTree *
InnerNode::clone() const {
  std::vector<StringTreeRef> v;
  for (auto child : children) {
    v.emplace_back(child->clone());
  }
  InnerNode *copy = new InnerNode(v);
  return copy;
}

void
InnerNode::flush(Stream &ss) const {
  for (auto child : children) {
    child->flush(ss);
  }
}

InnerNode *
InnerNode::lift() const {
  InnerNode *copy = new InnerNode(*this);
  return copy;
}

bool
TokenNode::classof(const StringTree *node) {
  return node->getKind() == StringTreeKind::Token;
}

TokenNode::TokenNode(const std::string &s)
    : StringTree(StringTreeKind::Token), token(s) {
}

StringTree *
TokenNode::clone() const {
  TokenNode *copy = new TokenNode(*this);
  return copy;
}

void
TokenNode::flush(Stream &ss) const {
  ss << token;
}

InnerNode *
TokenNode::lift() const {
  std::vector<StringTreeRef> v({std::make_shared<TokenNode>(token)});
  InnerNode *node = new InnerNode(v);
  return node;
}

void
InnerNode::amend(const StringTreeRef &node) {
  std::unique_ptr<InnerNode> IN(node->lift());
  std::copy(
      IN->children.begin(), IN->children.end(), std::back_inserter(children));
}

bool
NewLineNode::classof(const StringTree *node) {
  return node->getKind() == StringTreeKind::NewLine;
}

NewLineNode::NewLineNode() : StringTree(StringTreeKind::NewLine){};

StringTree *
NewLineNode::clone() const {
  return (StringTree *)this;
}

void
NewLineNode::flush(Stream &ss) const {
  ss << CXXCodeGen::newline;
}

InnerNode *
NewLineNode::lift() const {
  std::vector<StringTreeRef> v({getsingleton()});
  InnerNode *node = new InnerNode(v);
  return node;
}

static void
nop(void *) {
}

StringTreeRef
NewLineNode::getsingleton() {
  static NewLineNode singleton;
  static StringTreeRef singletonptr(&singleton, nop);
  return singletonptr;
}

bool
SourcePosNode::classof(const StringTree *node) {
  return node->getKind() == StringTreeKind::SourcePos;
}

SourcePosNode::SourcePosNode(std::string f, size_t l)
    : StringTree(StringTreeKind::SourcePos), filename(f), lineno(l){};

StringTree *
SourcePosNode::clone() const {
  return (StringTree *)this;
}

void
SourcePosNode::flush(Stream &ss) const {
  ss.setLineInfo(filename, lineno);
}

InnerNode *
SourcePosNode::lift() const {
  std::vector<StringTreeRef> v(
      {std::make_shared<SourcePosNode>(filename, lineno)});
  InnerNode *node = new InnerNode(v);
  return node;
}

StringTreeRef
makeInnerNode(const std::vector<StringTreeRef> &v) {
  return std::make_shared<InnerNode>(v);
}

StringTreeRef
makeNewLineNode() {
  return NewLineNode::getsingleton();
}

StringTreeRef
makeTokenNode(const std::string &s) {
  return std::make_shared<TokenNode>(s);
}

std::string
to_string(const StringTreeRef &str) {
  Stream ss;
  str->flush(ss);
  return ss.str();
}

StringTreeRef
makeVoidNode() {
  return std::make_shared<TokenNode>("");
}

namespace {
// helpers

StringTreeRef
wrapWithStr(const std::string &opening,
    const StringTreeRef &str,
    const std::string &closing) {
  return makeTokenNode(opening) + str + makeTokenNode(closing);
}

} // namespace

StringTreeRef
insertNewLines(const std::vector<StringTreeRef> &strs) {
  auto acc = makeVoidNode();
  for (auto &str : strs) {
    acc = acc + str + makeNewLineNode();
  }
  return acc;
}

StringTreeRef
separateByBlankLines(const std::vector<StringTreeRef> &strs) {
  auto acc = makeVoidNode();
  for (auto &str : strs) {
    acc = acc + str + makeNewLineNode() + makeNewLineNode();
  }
  return acc;
}

StringTreeRef
foldWithSemicolon(const std::vector<StringTreeRef> &stmts) {
  auto node = makeVoidNode();
  for (auto &stmt : stmts) {
    node = node + stmt + makeTokenNode(";") + makeNewLineNode();
  }
  return node;
}

StringTreeRef
join(const std::string &delim, const std::vector<StringTreeRef> &strs) {
  auto acc = makeVoidNode();
  bool alreadyPrinted = false;
  for (auto &str : strs) {
    if (alreadyPrinted) {
      acc = acc + makeTokenNode(delim);
    }
    acc = acc + str;
    alreadyPrinted = true;
  }
  return acc;
}

StringTreeRef
wrapWithParen(const StringTreeRef &str) {
  return wrapWithStr("(", str, ")");
}

StringTreeRef
wrapWithSquareBracket(const StringTreeRef &str) {
  return wrapWithStr("[", str, "]");
}

StringTreeRef
wrapWithBrace(const StringTreeRef &str) {
  return wrapWithStr("{", str, "}");
}

StringTreeRef
wrapWithXcodeMlIdentity(const StringTreeRef &type) {
  return makeTokenNode("__xcodeml_identity<") + type + makeTokenNode(">::t");
}

} // namespace CXXCodeGen

CXXCodeGen::StringTreeRef operator+(const CXXCodeGen::StringTreeRef &lhs,
    const CXXCodeGen::StringTreeRef &rhs) {
  auto ret = lhs->lift();
  ret->amend(rhs);
  return CXXCodeGen::StringTreeRef(ret);
}
