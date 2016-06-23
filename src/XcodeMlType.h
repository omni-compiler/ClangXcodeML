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

enum class XcodeMlTypeKind {
  Reserved,
  Pointer,
  Function,
  Array,
};

class XcodeMlType {
public:
private:
  class Impl;
  std::shared_ptr<Impl> impl;
};

