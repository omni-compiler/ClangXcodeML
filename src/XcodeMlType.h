#include <vector>
#include <memory>
#include <string>

class XcodeMlType;
using XcodeMlTypeRef = std::shared_ptr<XcodeMlType>; /* not nullable */

struct XcodeMlReservedType {
  std::string name;
};

struct XcodeMlPointerType {
  XcodeMlTypeRef ref;
};

struct XcodeMlFunctionType {
  XcodeMlTypeRef returnType;
  std::vector<XcodeMlTypeRef> params;
};

struct XcodeMlArrayType {
  XcodeMlTypeRef elementType;
  std::shared_ptr<size_t> size;
};

enum XcodeMlTypeKind {
  XTK_Reserved,
  XTK_Pointer,
  XTK_Function,
  XTK_Array,
};

class XcodeMlType {
public:
  void swap(XcodeMlType&);
  XcodeMlType& operator=(XcodeMlType);
private:
  class Impl;
  std::unique_ptr<Impl> impl;
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

XcodeMlType::Impl::~Impl() {
  switch (kind) {
    case XTK_None:
      break;
    case XTK_Reserved:
      reservedType.~XcodeMlReservedType();
    case XTK_Pointer:
      pointerType.~XcodeMlPointerType();
    case XTK_Function:
      functionType.~XcodeMlFunctionType();
    case XTK_Array:
      arrayType.~XcodeMlArrayType();
  }
}
