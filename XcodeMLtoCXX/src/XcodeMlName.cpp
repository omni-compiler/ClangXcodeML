#include <map>
#include <memory>
#include <string>
#include <vector>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "StringTree.h"
#include "XcodeMlType.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"

namespace XcodeMl {

UnqualId::UnqualId(UnqualIdKind k):
  kind(k)
{}

UnqualId::~UnqualId() {
}

UnqualIdKind
UnqualId::getKind() const {
  return kind;
}

UIDIdent::UIDIdent(const std::string& id):
  UnqualId(UnqualIdKind::Ident),
  ident(id)
{}

UnqualId*
UIDIdent::clone() const {
  auto copy = new UIDIdent(*this);
  return copy;
}

bool
UIDIdent::classof(const UnqualId* id) {
  return id->getKind() == UnqualIdKind::Ident;
}

OpFuncId::OpFuncId(const std::string& op):
  UnqualId(UnqualIdKind::OpFuncId),
  opName(op)
{}

UnqualId*
OpFuncId::clone() const {
  auto copy = new OpFuncId(*this);
  return copy;
}

bool
OpFuncId::classof(const UnqualId* id) {
  return id->getKind() == UnqualIdKind::OpFuncId;
}

ConvFuncId::ConvFuncId(const DataTypeIdent& type):
  UnqualId(UnqualIdKind::ConvFuncId),
  dtident(type)
{}

UnqualId*
ConvFuncId::clone() const {
  auto copy = new ConvFuncId(*this);
  return copy;
}

bool
ConvFuncId::classof(const UnqualId* id) {
  return id->getKind() == UnqualIdKind::ConvFuncId;
}

} // namespace XcodeMl
