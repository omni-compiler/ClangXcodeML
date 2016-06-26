#include <vector>
#include <memory>
#include <string>
#include <map>

class XcodeMlType;
using XcodeMlTypeRef = std::shared_ptr<XcodeMlType>; /* not nullable */
using TypeMap = std::map<std::string, XcodeMlTypeRef>;

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
  friend XcodeMlTypeKind typeKind(XcodeMlTypeRef);

  friend XcodeMlTypeRef makeReservedType(std::string);
  friend XcodeMlTypeRef makePointerType(XcodeMlTypeRef);
  friend XcodeMlTypeRef makeFunctionType(XcodeMlTypeRef, const std::vector<XcodeMlTypeRef>&);
  friend XcodeMlTypeRef makeArrayType(XcodeMlTypeRef, size_t);

  friend XcodeMlReservedType getReservedType(XcodeMlTypeRef);
  friend XcodeMlPointerType getPointerType(XcodeMlTypeRef);
  friend XcodeMlFunctionType getFunctionType(XcodeMlTypeRef);
  friend XcodeMlArrayType getArrayType(XcodeMlTypeRef);
private:
  XcodeMlTypeKind kind;
  /* FIXME: knows too much */
  std::string name;
  XcodeMlTypeRef type;
  std::vector<XcodeMlTypeRef> params;
  std::shared_ptr<size_t> size;
};
