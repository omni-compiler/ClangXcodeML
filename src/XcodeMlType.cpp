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
    XcodeMlReservedType reserved;
    XcodeMlPointerType pointer;
    XcodeMlFunctionType function;
    XcodeMlArrayType array;
  } type;
  Impl(const Impl& other) = default;
  Impl& operator=(Impl rhs);
  ~Impl();
  void swap(Impl&);
};

XcodeMlType::Impl& XcodeMlType::Impl::operator=(Impl rhs) {
  rhs.swap(*this);
  return *this;
}

XcodeMlType::Impl::~Impl() {
  switch (kind) {
    case XcodeMlTypeKind::Reserved:
      type.reserved.~XcodeMlReservedType();
      break;
    case XcodeMlTypeKind::Pointer:
      type.pointer.~XcodeMlPointerType();
      break;
    case XcodeMlTypeKind::Function:
      type.function.~XcodeMlFunctionType();
      break;
    case XcodeMlTypeKind::Array:
      type.array.~XcodeMlArrayType();
      break;
  }
}

void XcodeMlType::Impl::swap(Impl& other) {
  using std::swap;
  swap(kind, other.kind);
  swap(type, other.type);
}

