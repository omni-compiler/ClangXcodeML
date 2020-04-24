#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"

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

#include "CodeBuilder.h"

#include "ClangTypeLocHandler.h"

#define TYPELOCHANDLER_ARGS                                                   \
  xmlNodePtr node __attribute__((unused)),                                    \
      const CodeBuilder &w __attribute__((unused)),                           \
      SourceInfo &src __attribute__((unused))

#define DEFINE_TYPELOCHANDLER(name)                                           \
  XcodeMl::CodeFragment name(TYPELOCHANDLER_ARGS)

namespace {

DEFINE_TYPELOCHANDLER(callCodeBuilder) {
  return makeInnerNode(ProgramBuilder.walkChildren(node, src));
}

DEFINE_TYPELOCHANDLER(BuiltinTypeProc) {
  const auto dtident = getType(node);
  return makeDecl(src.typeTable.at(dtident),
      CXXCodeGen::makeVoidNode(),
      src.typeTable,
      src.nnsTable);
}

} // namespace

const ClangTypeLocHandlerType ClangTypeLocHandler("class",
    CXXCodeGen::makeInnerNode,
    callCodeBuilder,
    {
        std::make_tuple("Builtin", BuiltinTypeProc),
    });
