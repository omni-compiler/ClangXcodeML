#include <vector>
#include <memory>
#include <string>

struct XcodeMlType;
using RefXcodeMlType = std::unique_ptr<XcodeMlType>; /* not nullable */

struct XcodeMlReservedType {
  std::string name;
};

struct XcodeMlPointerType {
  RefXcodeMlType ref;
};

struct XcodeMlFunctionType {
  RefXcodeMlType returnType;
  std::vector<RefXcodeMlType> params;
};

struct XcodeMlArrayType {
  RefXcodeMlType elementType;
  std::unique_ptr<size_t> size;
};

enum XcodeMlTypeKind {
  XTK_Reserved,
  XTK_Pointer,
  XTK_Function,
  XTK_Array,
};

struct XcodeMlType {
  XcodeMlTypeKind kind;
  union type {
    XcodeMlReservedType reserved;
    XcodeMlPointerType pointer;
    XcodeMlFunctionType function;
    XcodeMlArrayType array;
  };
};
