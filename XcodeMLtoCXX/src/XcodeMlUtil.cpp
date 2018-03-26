#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <libxml/debugXML.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "LibXMLUtil.h"
#include "StringTree.h"
#include "Util.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlTypeTable.h"
#include "XcodeMlName.h"
#include "XcodeMlOperator.h"
#include "XMLString.h"
#include "SourceInfo.h"
#include "AttrProc.h"
#include "XMLWalker.h"
#include "ClangNestedNameSpecHandler.h"

#include "XcodeMlUtil.h"

std::shared_ptr<XcodeMl::UnqualId>
getUnqualIdFromNameNode(xmlNodePtr nameNode) {
  const auto kind = getProp(nameNode, "name_kind");

  if (kind == "constructor") {
    const auto dtident = getProp(nameNode, "ctor_type");
    return std::make_shared<XcodeMl::CtorName>(dtident);
  } else if (kind == "destructor") {
    const auto dtident = getProp(nameNode, "dtor_type");
    return std::make_shared<XcodeMl::DtorName>(dtident);
  } else if (kind == "operator") {
    const auto opName = getContent(nameNode);
    const auto pOpId = XcodeMl::OperatorNameToSpelling(opName);
    assert(pOpId.hasValue());
    return std::make_shared<XcodeMl::OpFuncId>(*pOpId);
  } else if (kind == "conversion") {
    const auto dtident = getProp(nameNode, "destination_type");
    return std::make_shared<XcodeMl::ConvFuncId>(dtident);
  }

  assert(kind == "name");
  const auto name = getContent(nameNode);
  return std::make_shared<XcodeMl::UIDIdent>(name);
}

std::shared_ptr<XcodeMl::UnqualId>
getUnqualIdFromIdNode(xmlNodePtr idNode, xmlXPathContextPtr ctxt) {
  if (!idNode) {
    throw std::domain_error("expected id node, but got null");
  }
  xmlNodePtr nameNode = findFirst(idNode, "name", ctxt);
  if (!nameNode) {
    return std::make_shared<XcodeMl::UnnamedId>();
    // throw std::domain_error("name node not found");
  }
  return getUnqualIdFromNameNode(nameNode);
}

XcodeMl::Name
getQualifiedName(xmlNodePtr node, const SourceInfo &src) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  assert(nameNode);
  const auto unqualId = getUnqualIdFromNameNode(nameNode);

  const auto nameSpecNode =
      findFirst(node, "clangNestedNameSpecifier", src.ctxt);
  if (!nameSpecNode) {
    return XcodeMl::Name(unqualId);
  }
  const auto nameSpec = ClangNestedNameSpecHandler.walk(nameSpecNode, src);
  return XcodeMl::Name(nameSpec, unqualId);
}

void
xcodeMlPwd(xmlNodePtr node, std::ostream &os) {
  assert(node);
  if (!(node->parent)) {
    os << "/";
    return;
  }

  xcodeMlPwd(node->parent, os);
  const auto name = getName(node);
  const auto comment = static_cast<std::string>("(:")
      + std::to_string(xmlGetLineNo(node)) + static_cast<std::string>(":)");

  const std::map<std::string, std::string> specialNodes = {
      {"clangStmt", "class"}, {"clangDecl", "class"},
  };
  const auto iter = specialNodes.find(name);
  if (iter == specialNodes.end()) {
    os << name << comment << "/";
    return;
  }
  const auto attrVal = getPropOrNull(node, iter->second);
  if (!attrVal.hasValue()) {
    os << name << comment << "(:no @" << iter->second << ":)/";
    return;
  }
  os << name << "[@" << iter->second << "='" << *attrVal << "']" << comment
     << "/";
}

XcodeMlPwdType
getXcodeMlPath(xmlNodePtr node) {
  return {node};
}

std::vector<XcodeMl::CodeFragment>
getParamNames(xmlNodePtr fnNode, const SourceInfo &src) {
  std::vector<XcodeMl::CodeFragment> vec;
  const auto params = findNodes(
      fnNode, "clangTypeLoc/clangDecl[@class='ParmVar']/name", src.ctxt);
  for (auto p : params) {
    XMLString name = xmlNodeGetContent(p);
    vec.push_back(CXXCodeGen::makeTokenNode(name));
  }
  return vec;
}

XcodeMl::CodeFragment
makeFunctionDeclHead(XcodeMl::Function *func,
    const XcodeMl::Name &name,
    const std::vector<XcodeMl::CodeFragment> &paramNames,
    const SourceInfo &src,
    bool emitNameSpec) {
  const auto pUnqualId = name.getUnqualId();
  const auto nameSpelling = emitNameSpec
      ? name.toString(src.typeTable, src.nnsTable)
      : pUnqualId->toString(src.typeTable, src.nnsTable);
  if (llvm::isa<XcodeMl::CtorName>(pUnqualId.get())
      || llvm::isa<XcodeMl::DtorName>(pUnqualId.get())
      || llvm::isa<XcodeMl::ConvFuncId>(pUnqualId.get())) {
    /* Do not emit return type
     *    void A::A();
     *    void A::~A();
     *    int A::operator int();
     */
    return func->makeDeclarationWithoutReturnType(
        nameSpelling, paramNames, src.typeTable, src.nnsTable);
  } else {
    return func->makeDeclaration(
        nameSpelling, paramNames, src.typeTable, src.nnsTable);
  }
}

std::string
getType(xmlNodePtr node) {
  const auto type = getPropOrNull(node, "xcodemlType");
  if (type.hasValue()) {
    return *type;
  }

  return getProp(node, "type");
}

XcodeMl::CodeFragment
makeFunctionDeclHead(xmlNodePtr node,
    const std::vector<XcodeMl::CodeFragment> paramNames,
    const SourceInfo &src,
    bool emitNameSpec) {
  const auto name = getQualifiedName(node, src);

  const auto dtident = getType(node);
  const auto T = src.typeTable[dtident];
  const auto fnType = llvm::cast<XcodeMl::Function>(T.get());

  auto acc = isTrueProp(node, "is_function_template_specialization", false)
      ? CXXCodeGen::makeTokenNode("template <>")
      : CXXCodeGen::makeVoidNode();
  acc = acc + makeFunctionDeclHead(fnType,
                  name,
                  paramNames,
                  src,
                  emitNameSpec && xmlHasProp(node, BAD_CAST "parent_class"));
  return acc;
}

bool
requiresSemicolon(xmlNodePtr node, const SourceInfo &src) {
  const auto declClass = getProp(node, "class");

  const std::vector<std::string> fnDecls = {
      "Function",
      "CXXConstructor",
      "CXXConversion",
      "CXXDestructor",
      "CXXMethod",
  };
  if (std::find(fnDecls.begin(), fnDecls.end(), declClass) != fnDecls.end()) {
    /*
     * A function (or member function) declaration ends with a semicolon
     * if it is not a definition.
     */
    return !findFirst(node, "clangStmt", src.ctxt);
  }

  const std::vector<std::string> Decls = {
      "AccessSpec",
      "FunctionTemplate",
      /*
       * `clangStmt[@class='FunctionTemplate']` itself doesn't require
       * semicolons. `clangStmt[@class='FunctionTemplate'] /
       * clangStmt[@class='Function']` does.
       */
      "LinkageSpec",
      "Namespace",
  };
  return std::find(Decls.begin(), Decls.end(), declClass) == Decls.end();
}

XcodeMl::CodeFragment
wrapWithLangLink(const XcodeMl::CodeFragment &content,
    xmlNodePtr node,
    const SourceInfo &src) {
  using namespace CXXCodeGen;
  if (src.language != Language::CPlusPlus) {
    return content;
  }
  const auto lang = getPropOrNull(node, "language_linkage");
  if (!lang.hasValue() || *lang == "C++") {
    return content;
  } else {
    return makeTokenNode("extern") + makeTokenNode("\"" + *lang + "\"")
        + wrapWithBrace(
               content + (requiresSemicolon(node, src) ? makeTokenNode(";")
                                                       : makeVoidNode()));
  }
}

std::ostream &operator<<(std::ostream &os, const XcodeMlPwdType &x) {
  xcodeMlPwd(x.node, os);
  return os;
}
