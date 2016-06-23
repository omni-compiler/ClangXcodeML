#include "XcodeMlType.h"

XcodeMlReservedType::XcodeMlReservedType(std::string n):
  name(n)
{}

XcodeMlPointerType::XcodeMlPointerType(XcodeMlTypeRef t):
  ref(t)
{}

XcodeMlFunctionType::XcodeMlFunctionType(
    XcodeMlTypeRef r,
    const std::vector<XcodeMlTypeRef>& p
  ):
  returnType(r),
  params(p)
{}

XcodeMlArrayType::XcodeMlArrayType(XcodeMlTypeRef t, size_t s):
  elementType(t),
  size(std::make_shared<size_t>(s))
{}

class XcodeMlType::Impl {
public:
  XcodeMlTypeKind kind;
  union {
    XcodeMlReservedType reservedType;
    XcodeMlPointerType pointerType;
    XcodeMlFunctionType functionType;
    XcodeMlArrayType arrayType;
  };
  ~Impl();
};

void XcodeMlType::swap(XcodeMlType& other) {
  std::swap(this->impl, other.impl);
}

namespace std {
  template<>
  void swap(XcodeMlType& lhs, XcodeMlType& rhs) {
    lhs.swap(rhs);
  }
}

XcodeMlType& XcodeMlType::operator=(XcodeMlType rhs) {
  rhs.swap(*this);
  return *this;
}

XcodeMlType::Impl::~Impl() {
  switch (kind) {
    case XcodeMlTypeKind::Reserved:
      reservedType.~XcodeMlReservedType();
      break;
    case XcodeMlTypeKind::Pointer:
      pointerType.~XcodeMlPointerType();
      break;
    case XcodeMlTypeKind::Function:
      functionType.~XcodeMlFunctionType();
      break;
    case XcodeMlTypeKind::Array:
      arrayType.~XcodeMlArrayType();
      break;
  }
}
