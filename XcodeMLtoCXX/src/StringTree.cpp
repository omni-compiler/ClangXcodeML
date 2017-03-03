#include <memory>
#include <sstream>
#include <vector>

#include "llvm/Support/Casting.h"

#include "StringTree.h"

namespace CXXCodeGen {

StringTree::StringTree(StringTreeKind k):
  kind(k)
{}

StringTree::~StringTree() {
}

StringTreeKind
StringTree::getKind() const {
  return kind;
}

bool
InnerNode::classof(const StringTree* node) {
  return node->getKind() == StringTreeKind::Inner;
}

InnerNode::InnerNode(const std::vector<StringTreeRef>& v):
  StringTree(StringTreeKind::Inner),
  children(v)
{}

StringTree*
InnerNode::clone() const {
  std::vector<StringTreeRef> v;
  for (auto child : children) {
    v.emplace_back(child->clone());
  }
  InnerNode* copy = new InnerNode(v);
  return copy;
}

void
InnerNode::flush(std::stringstream& ss) const {
  for (auto child : children) {
    child->flush(ss);
  }
}

InnerNode*
InnerNode::lift() const {
  InnerNode* copy = new InnerNode(*this);
  return copy;
}

bool
TokenNode::classof(const StringTree* node) {
  return node->getKind() == StringTreeKind::Token;
}

TokenNode::TokenNode(const std::string& s):
  StringTree(StringTreeKind::Token),
  token(s)
{}

StringTree*
TokenNode::clone() const {
  TokenNode* copy = new TokenNode(*this);
  return copy;
}

void
TokenNode::flush(std::stringstream& ss) const {
  ss << token;
}

InnerNode*
TokenNode::lift() const {
  std::vector<StringTreeRef> v({
      std::make_shared<TokenNode>(token)
      });
  InnerNode* node = new InnerNode(v);
  return node;
}

void
InnerNode::amend(const StringTreeRef& node) {
  std::unique_ptr<InnerNode> IN( node->lift() );
  std::copy(
      IN->children.begin(), IN->children.end(),
      std::back_inserter(children));
}

StringTreeRef
makeInnerNode(const std::vector<StringTreeRef>& v) {
  return std::make_shared<InnerNode>(v);
}

StringTreeRef
makeTokenNode(const std::string& s) {
  return std::make_shared<TokenNode>(s);
}

CXXCodeGen::StringTreeRef operator+(
    const StringTreeRef& lhs,
    const StringTreeRef& rhs)
{
  auto ret = lhs->lift();
  ret->amend(rhs);
  return StringTreeRef( ret );
}

} // namespace CXXCodeGen
