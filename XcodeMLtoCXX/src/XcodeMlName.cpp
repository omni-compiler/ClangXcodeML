#include <string>
#include "XcodeMlType.h"
#include "XcodeMlNns.h"
#include "XcodeMlName.h"

namespace XcodeMl {

UnqualId::~UnqualId() {
}

Ident::Ident(const std::string& id):
  UnqualId(UnqualIdKind::Ident),
  ident(id)
{}

Ident::

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

ConvFuncId::clone() const {
  auto copy = new ConvFuncId(*this);
  return copy;
}

bool
ConvFuncId::classof(const UnqualId* id) {
  return id->getKind() == UnqualIdKind::ConvFuncId;
}

} // namespace XcodeMl
