#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"

#include "LibXMLUtil.h"
#include "StringTree.h"
#include "XcodeMlNns.h"
#include "XMLString.h"

#include "XcodeMlName.h"

#include "XcodeMlType.h"
#include "XcodeMlUtil.h"
#include "XcodeMlTypeTable.h"

#include "AttrProc.h"
#include "SourceInfo.h"
#include "XMLWalker.h"

#include "ClangNestedNameSpecHandler.h"

using CXXCodeGen::makeTokenNode;

#define NAMESPECHANDLER_ARGS                                                  \
  xmlNodePtr node __attribute__((unused)),                                    \
      const SourceInfo &src __attribute__((unused))

#define DEFINE_NAMESPECHANDLER(name)                                          \
  XcodeMl::CodeFragment name(NAMESPECHANDLER_ARGS)

namespace {

DEFINE_NAMESPECHANDLER(doNothing) {
  return CXXCodeGen::makeVoidNode();
}

DEFINE_NAMESPECHANDLER(globalSpecProc) {
  return makeTokenNode("::");
}

DEFINE_NAMESPECHANDLER(NamespaceSpecProc) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto name = getUnqualIdFromNameNode(nameNode)->toString(src.typeTable);
  if (const auto parent =
          findFirst(node, "clangNestedNameSpecifier", src.ctxt)) {
    const auto prefix = ClangNestedNameSpecHandler.walk(parent, src);
    return prefix + name + makeTokenNode("::");
  }
  return name + makeTokenNode("::");
}

DEFINE_NAMESPECHANDLER(TypeSpecifierProc) {
  const auto typeNode = findFirst(node, "clangTypeLoc", src.ctxt);
  const auto T = src.typeTable.at(getType(typeNode));
  if (llvm::isa<XcodeMl::ClassType>(T.get())) {
    const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
    return classT->name() + makeTokenNode("::");
  }
  return makeDecl(T, CXXCodeGen::makeVoidNode(), src.typeTable, src.nnsTable)
      + makeTokenNode("::");
}

} // namespace

const ClangNestedNameSpecHandlerType ClangNestedNameSpecHandler(
    "clang_nested_name_specifier_kind",
    CXXCodeGen::makeInnerNode,
    doNothing,
    {
        std::make_tuple("global", globalSpecProc),
        std::make_tuple("namespace", NamespaceSpecProc),
        std::make_tuple("type_specifier", TypeSpecifierProc),
    });
