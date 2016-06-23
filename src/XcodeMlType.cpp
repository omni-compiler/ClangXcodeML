#include "XcodeMlType.h"

class XcodeMlType::Impl {
public:
  XcodeMlTypeKind kind;
  union {
    XcodeMlReservedType reservedType;
    XcodeMlPointerType pointerType;
    XcodeMlFunctionType functionType;
    XcodeMlArrayType arrayType;
  };
  Impl& operator=(Impl&& rhs);
  Impl& operator=(Impl const & rhs);
  ~Impl();
};

XcodeMlType::Impl& XcodeMlType::Impl::operator=(Impl&& rhs) {
  switch (rhs.kind) {
    case XcodeMlTypeKind::Reserved:
      reservedType = rhs.reservedType;
      break;
    case XcodeMlTypeKind::Pointer:
      pointerType = rhs.pointerType;
      break;
    case XcodeMlTypeKind::Function:
      functionType = rhs.functionType;
      break;
    case XcodeMlTypeKind::Array:
      arrayType = rhs.arrayType;
      break;
  }
  return *this;
}

XcodeMlType::Impl& XcodeMlType::Impl::operator=(Impl const& rhs) {
  return *this = std::move(rhs);
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

