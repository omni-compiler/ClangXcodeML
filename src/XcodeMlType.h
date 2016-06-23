#include <vector>
#include <memory>
#include <string>

class XcodeMlType;
using XcodeMlTypeRef = std::shared_ptr<XcodeMlType>; /* not nullable */

class XcodeMlReservedType {
public:
  XcodeMlReservedType(std::string);
private:
  std::string name;
};

class XcodeMlPointerType {
public:
  XcodeMlPointerType(XcodeMlTypeRef);
private:
  XcodeMlTypeRef ref;
};

class XcodeMlFunctionType {
public:
  XcodeMlFunctionType(XcodeMlTypeRef, const std::vector<XcodeMlTypeRef>&);
private:
  XcodeMlTypeRef returnType;
  std::vector<XcodeMlTypeRef> params;
};

class XcodeMlArrayType {
public:
  XcodeMlArrayType(XcodeMlTypeRef, size_t);
private:
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
  void swap(XcodeMlType&);
  XcodeMlType& operator=(XcodeMlType);
private:
  class Impl;
  std::unique_ptr<Impl> impl;
};

