#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <string>
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
  const auto name =
      getUnqualIdFromNameNode(nameNode)->toString(src.typeTable, src.nnsTable);
  if (const auto parent =
          findFirst(node, "clangNestedNameSpecifier", src.ctxt)) {
    const auto prefix = ClangNestedNameSpecHandler.walk(parent, src);
    return prefix + name + makeTokenNode("::");
  }
  return name + makeTokenNode("::");
}

XcodeMl::CodeFragment
makeTypeNameSpecifier(const XcodeMl::TypeRef &T, const SourceInfo &src) {
  using namespace XcodeMl;
  switch (T->getKind()) {
  case TypeKind::Class: {
    const auto classT = llvm::cast<ClassType>(T.get());
    return classT->name() + makeTokenNode("::");
  }
  case TypeKind::TemplateTypeParm: {
    const auto TTPT = llvm::cast<TemplateTypeParm>(T.get());
    const auto name = TTPT->getSpelling().getValue();
    return name + makeTokenNode("::");
  }
  default: {
    const auto decl =
        makeDecl(T, CXXCodeGen::makeVoidNode(), src.typeTable, src.nnsTable);
    return decl + makeTokenNode("::");
  }
  }
}

DEFINE_NAMESPECHANDLER(TypeSpecifierProc) {
  const auto typeNode = findFirst(node, "clangTypeLoc", src.ctxt);
  const auto T = src.typeTable.at(getType(typeNode));
  const auto spec = makeTypeNameSpecifier(T, src);
  if (const auto parent =
          findFirst(node, "clangNestedNameSpecifier", src.ctxt)) {
    const auto prefix = ClangNestedNameSpecHandler.walk(parent, src);
    return prefix + spec;
  }
  return spec;
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
