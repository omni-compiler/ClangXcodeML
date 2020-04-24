#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include <exception>
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
#include "NnsAnalyzer.h"
#include "SourceInfo.h"
#include "TypeAnalyzer.h"
#include "XMLWalker.h"

#include "CodeBuilder.h"

#include "ClangDeclHandler.h"

using XcodeMl::CodeFragment;
using CXXCodeGen::makeTokenNode;

#define DECLHANDLER_ARGS                                                      \
  xmlNodePtr node __attribute__((unused)),                                    \
      const CodeBuilder &w __attribute__((unused)),                           \
      SourceInfo &src __attribute__((unused))

#define DEFINE_DECLHANDLER(name) XcodeMl::CodeFragment name(DECLHANDLER_ARGS)

namespace {

std::vector<CodeFragment>
createNodes(xmlNodePtr node,
    const char *xpath,
    const CodeBuilder &w,
    SourceInfo &src) {
  std::vector<CodeFragment> vec;
  const auto targetNodes = findNodes(node, xpath, src.ctxt);
  for (auto &&targetNode : targetNodes) {
    vec.push_back(w.walk(targetNode, src));
  }
  return vec;
}

CodeFragment
foldDecls(xmlNodePtr node, const CodeBuilder &w, SourceInfo &src) {
  const auto declNodes = findNodes(node, "clangDecl", src.ctxt);
  std::vector<CodeFragment> decls;
  for (auto &&declNode : declNodes) {
    if (isTrueProp(declNode, "is_implicit", false)) {
      continue;
    }
    const auto decl = w.walk(declNode, src);

    if (requiresSemicolon(declNode, src)) {
      decls.push_back(decl + makeTokenNode(";"));
    } else {
      decls.push_back(decl);
    }
  }
  return insertNewLines(decls);
}

CodeFragment
makeMemberInitList(xmlNodePtr node, SourceInfo &src) {
  const auto initNodes =
      findNodes(node, "clangConstructorInitializer", src.ctxt);
  if (initNodes.empty()) {
    return CXXCodeGen::makeVoidNode();
  }

  bool first = true;
  std::vector<CodeFragment> inits;
  for (auto &&initNode : initNodes) {
    inits.push_back(ProgramBuilder.walk(initNode, src));
    first = false;
  }

  return makeTokenNode(":") + join(",", inits);
}

DEFINE_DECLHANDLER(callCodeBuilder) {
  return makeInnerNode(ProgramBuilder.walkChildren(node, src));
}

  CodeFragment
makeVariableArraySize(xmlNodePtr node, SourceInfo &src,
		      const CodeBuilder &w)
{
  const auto dtident = getType(node);
  const auto T = src.typeTable[dtident];
  auto decl = CXXCodeGen::makeVoidNode();
  const auto arrayT = llvm::dyn_cast<XcodeMl::Array>(T.get());
  if(arrayT && !arrayT->isFixedSize()){
    auto TL = findFirst(node, "clangTypeLoc", src.ctxt);
    auto STMT = findFirst(TL, "clangStmt", src.ctxt);
    if(STMT != nullptr){
      auto arg = w.walk(STMT, src);
      decl = decl +  wrapWithSquareBracket(arg);
    }else{
      CXXCodeGen::makeTokenNode("[*]");
    }
  }

  return decl;
}

CodeFragment
makeTemplateHead(xmlNodePtr node, const CodeBuilder &w, SourceInfo &src) {
  const auto paramNodes =
      findNodes(node, "clangDecl[@class='TemplateTypeParm' or @class ='NonTypeTemplateParm']", src.ctxt);
  std::vector<CXXCodeGen::StringTreeRef> params;
  for (auto &&paramNode : paramNodes) {
    params.push_back(w.walk(paramNode, src));
  }
  return makeTokenNode("template") + makeTokenNode("<") + join(",", params)
      + makeTokenNode(">");
}


XcodeMl::CodeFragment
makeBases(const XcodeMl::ClassType &T, SourceInfo &src) {
  using namespace XcodeMl;
  const auto bases = T.getBases();
  std::vector<CodeFragment> decls;
  std::transform(bases.begin(),
      bases.end(),
      std::back_inserter(decls),
      [&src](ClassType::BaseClass base) {
        const auto T = src.typeTable.at(std::get<1>(base));
	//std::cerr <<std::get<0>(base)<<","<<std::get<1>(base)<<std::endl;
	CodeFragment desc;
	if(const auto classT = llvm::dyn_cast<ClassType>(T.get())){
	  desc = classT->getAsTemplateId(src.typeTable, src.nnsTable)
	    .getValueOr(classT->name());
	}else if(const auto TS = llvm::dyn_cast<TemplateSpecializationType>(T.get())){
	  desc = TS->makeDeclaration(CXXCodeGen::makeVoidNode(),
			     src.typeTable, src.nnsTable);
	}else if(const auto TT = llvm::dyn_cast<TemplateTypeParm>(T.get())){
	  desc = TT->makeDeclaration(CXXCodeGen::makeVoidNode(),
				     src.typeTable, src.nnsTable);
	}else if(const auto DN = llvm::dyn_cast<DependentNameType>(T.get())){
	  desc = CXXCodeGen::makeTokenNode("/**/");
	}else if(const auto DT = llvm::dyn_cast<DeclType>(T.get())){
	  desc = CXXCodeGen::makeTokenNode("/**/");
	}else{
	  throw std::runtime_error("Cannot interpret base");
	}

        return makeTokenNode(std::get<0>(base))
            + makeTokenNode(std::get<2>(base) ? "virtual" : "")
            + desc;
      });
  return decls.empty() ? CXXCodeGen::makeVoidNode()
                       : makeTokenNode(":") + CXXCodeGen::join(",", decls);
}

bool
isTemplateParam(xmlNodePtr node) {
  const auto name = getName(node);
  if (!std::equal(name.begin(), name.end(), "clangDecl")) {
    return false;
  }
  const auto kind = getProp(node, "class");
  return std::equal(kind.begin(), kind.end(), "TemplateTypeParm")
      || std::equal(kind.begin(), kind.end(), "NonTypeTemplateParm")
      || std::equal(kind.begin(), kind.end(), "TemplateTemplateParm");
}

CodeFragment
emitClassDefinition(xmlNodePtr node,
    const CodeBuilder &w,
    SourceInfo &src,
    const XcodeMl::ClassType &classType) {
  if (isTrueProp(node, "is_implicit", false)) {
    return CXXCodeGen::makeVoidNode();
  }

  const auto memberNodes = findNodes(node, "clangDecl", src.ctxt);
  std::vector<XcodeMl::CodeFragment> decls;
  for (auto &&memberNode : memberNodes) {
    if (isTrueProp(memberNode, "is_implicit", false)
        || isTemplateParam(memberNode)) {
      continue;
    }
    /* Traverse `memberNode` regardless of whether `CodeBuilder` prints it. */
    const auto decl = w.walk(memberNode, src)
        + (requiresSemicolon(memberNode, src) ? makeTokenNode(";")
                                              : CXXCodeGen::makeVoidNode());

    const auto accessProp = getPropOrNull(memberNode, "access");
    if (accessProp.hasValue()) {
      const auto access = *accessProp;
      decls.push_back(makeTokenNode(access) + makeTokenNode(":") + decl);
    } else {
      decls.push_back(makeTokenNode(
	       "\n/* Ignored a member with no access specifier ") + decl +
		      makeTokenNode("*/\n"));
    }
  }

  const auto classKey = makeTokenNode(getClassKey(classType.classKind()));
  const auto name = classType.isClassTemplateSpecialization()
      ? classType.getAsTemplateId(src.typeTable, src.nnsTable).getValue()
      : classType.name();

  return classKey + name + makeBases(classType, src)
      + wrapWithBrace(insertNewLines(decls)) + CXXCodeGen::makeNewLineNode();
}

DEFINE_DECLHANDLER(ClassTemplateProc) {
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandTypeTable(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsTable(src.nnsTable, nnsTableNode, src.ctxt);
  }
  const auto bodyNode =
    findFirst(node, "clangDecl[@class='CXXRecord']", src.ctxt);
  //const auto bodyNode = records[0];

  const auto head = makeTemplateHead(node, w, src);
  const auto body = w.walk(bodyNode, src) + makeTokenNode(";");
  const auto specs = findNodes(node, "clangDecl[@class='ClassTemplateSpecialization']", src.ctxt);
  auto speccode = CXXCodeGen::makeTokenNode("\n");
  for(auto && ent : specs){
    speccode = speccode +  w.walk(ent, src);
  }
  return head + body + speccode;
}

DEFINE_DECLHANDLER(ClassTemplatePartialSpecializationProc) {
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandTypeTable(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsTable(src.nnsTable, nnsTableNode, src.ctxt);
  }

  const auto T = src.typeTable.at(getType(node));
  const auto classT = llvm::cast<XcodeMl::ClassType>(T.get());
  const auto head = makeTemplateHead(node, w, src);
  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    const auto def =
        emitClassDefinition(node, ClassDefinitionBuilder, src, *classT);
    return head + def;
  }
  /* forward declaration */
  const auto classKey = getClassKey(classT->classKind());
  const auto nameSpelling = classT->name();
  return head + makeTokenNode(classKey) + nameSpelling;
}

DEFINE_DECLHANDLER(ClassTemplateSpecializationProc) {
  const auto T = src.typeTable.at(getType(node));
  const auto classT = llvm::dyn_cast<XcodeMl::ClassType>(T.get());
  const auto nameSpelling = classT->name();

  const auto head =
      makeTokenNode("template") + makeTokenNode("<") + makeTokenNode(">");

  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    return head
        + emitClassDefinition(node, ClassDefinitionBuilder, src, *classT);
  }

  /* forward declaration */
  const auto classKey = getClassKey(classT->classKind());
  return head + makeTokenNode(classKey) + nameSpelling;
}

void
setClassName(XcodeMl::ClassType &classType, SourceInfo &src) {
  if (classType.name()) {
    return;
  }
  /* `classType` is unnamed.
   * Unnamed classes are problematic, so give a name to `classType`
   * such as `__xcodeml_1`.
   */
  classType.setName(src.getUniqueName());
}

DEFINE_DECLHANDLER(CXXRecordProc) {
  if (isTrueProp(node, "is_implicit", false)) {
    return CXXCodeGen::makeVoidNode();
  }

  const auto T = src.typeTable.at(getType(node));
  auto classT = llvm::dyn_cast<XcodeMl::ClassType>(T.get());
  //std::cerr <<getType(node)<<classT<<std::endl;
  assert(classT);
  setClassName(*classT, src);
  const auto nameSpelling = classT->name(); // now class name must exist

  if (isTrueProp(node, "is_this_declaration_a_definition", false)) {
    return emitClassDefinition(node, ClassDefinitionBuilder, src, *classT);
  }

  /* forward declaration */
  const auto classKey = getClassKey(classT->classKind());
  return makeTokenNode(classKey) + nameSpelling;
}

DEFINE_DECLHANDLER(emitInlineMemberFunction) {
  if (isTrueProp(node, "is_implicit", 0)) {
    return CXXCodeGen::makeVoidNode();
  }

  auto acc = CXXCodeGen::makeVoidNode();
  if (isTrueProp(node, "is_virtual", false)) {
    acc = acc + makeTokenNode("virtual");
  }
  if (isTrueProp(node, "is_static", false)) {
    acc = acc + makeTokenNode("static");
  }
  const auto paramNames = getParamNames(node, src);
  acc = acc + makeFunctionDeclHead(node, paramNames, src);
  acc = acc + makeMemberInitList(node, src);

  if (const auto bodyNode = findFirst(node, "clangStmt", src.ctxt)) {
    const auto body = ProgramBuilder.walk(bodyNode, src);
    return acc + body;
  } else if (isTrueProp(node, "is_pure", false)) {
    return acc + makeTokenNode("=") + makeTokenNode("0");
  }
  return acc;
}

DEFINE_DECLHANDLER(EnumProc) {
  std::vector<CodeFragment> decls;
  const auto children =
      findNodes(node, "clangDecl[@class='EnumConstant']", src.ctxt);
  for (auto &&child : children) {
    const auto decl = w.walk(child, src);
    decls.push_back(decl);
  }
  const auto dtident = getType(node);
  const auto T = src.typeTable.at(dtident);
  const auto enumT = llvm::cast<XcodeMl::EnumType>(T.get());
  const auto tagname = enumT->name();
  const auto nameSpelling = tagname
      ? tagname->toString(src.typeTable, src.nnsTable)
      : CXXCodeGen::makeVoidNode();

  return makeTokenNode("enum") + nameSpelling
      + wrapWithBrace(join(",", decls));
}

DEFINE_DECLHANDLER(EnumConstantProc) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto name =
      getUnqualIdFromNameNode(nameNode)->toString(src.typeTable, src.nnsTable);

  const auto exprNode = findFirst(node, "clangStmt", src.ctxt);
  if (!exprNode) {
    return name;
  }
  const auto expr = ProgramBuilder.walk(exprNode, src);
  return name + makeTokenNode("=") + expr;
}

DEFINE_DECLHANDLER(FieldDeclProc) {
  const auto dtident = getType(node);
  const auto T = src.typeTable.at(dtident);
  auto name = CXXCodeGen::makeVoidNode();
  auto bits = CXXCodeGen::makeVoidNode();
  if (isTrueProp(node, "is_bit_field", false)) {
    const auto bitsNode = findFirst(node, "clangStmt", src.ctxt);
    bits = makeTokenNode(":") + w.walk(bitsNode, src);
  }else{
    bits = makeVariableArraySize(node, src, w);
  }
  if (!isTrueProp(node, "is_unnamed_bit_field", false)) {
    const auto nameNode = findFirst(node, "name", src.ctxt);
    name = getUnqualIdFromNameNode(nameNode)->toString(
        src.typeTable, src.nnsTable);
  }
  return makeDecl(T, name, src.typeTable, src.nnsTable) + bits;
}

DEFINE_DECLHANDLER(FriendDeclProc) {
  if (auto TL = findFirst(node, "clangTypeLoc", src.ctxt)) {
    /* friend class declaration */
    const auto dtident = getType(TL);
    const auto T = src.typeTable.at(dtident);
    return makeTokenNode("friend")
        + makeDecl(T, CXXCodeGen::makeVoidNode(), src.typeTable, src.nnsTable);
  }
  return makeTokenNode("friend") + callCodeBuilder(node, w, src);
}

DEFINE_DECLHANDLER(FunctionProc) {
  if (isTrueProp(node, "is_implicit", 0)) {
    return CXXCodeGen::makeVoidNode();
  }
  const auto type = getProp(node, "xcodemlType");
  const auto paramNames = getParamNames(node, src);
  auto acc = makeFunctionDeclHead(node, paramNames, src, true);
  acc = acc + makeMemberInitList(node, src);

  if (const auto bodyNode = findFirst(node, "clangStmt", src.ctxt)) {
    const auto body = w.walk(bodyNode, src);
    acc = acc + body;
  }

  return wrapWithLangLink(acc, node, src);
}

DEFINE_DECLHANDLER(FunctionTemplateProc)
{
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandTypeTable(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsTable(src.nnsTable, nnsTableNode, src.ctxt);
  }
  const auto paramNodes =
      findNodes(node, "clangDecl[@class='TemplateTypeParm' or @class ='NonTypeTemplateParm']", src.ctxt);
  const auto bodyNodes = findNodes(node,
			      "clangDecl[@class='CXXMethod' or @class ='CXXConstructor' or @class='Function']"
			      , src.ctxt);
  std::vector<CXXCodeGen::StringTreeRef> params;
  for (auto &&paramNode : paramNodes) {
    params.push_back(w.walk(paramNode, src));
  }
  std::vector<CXXCodeGen::StringTreeRef> bodies;
  for(auto && bodyNode : bodyNodes){
    bodies.push_back(w.walk(bodyNode, src));    
  }
  return makeTokenNode("template") + makeTokenNode("<") + join(",", params)
    + makeTokenNode(">") + join("\n", bodies );

}

DEFINE_DECLHANDLER(LinkageSpecProc) {
  // We emit linkage specification by `wrapWithLangLink`
  // not here
  return foldDecls(node, w, src);
}

DEFINE_DECLHANDLER(NamespaceProc) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto name =
      getUnqualIdFromNameNode(nameNode)->toString(src.typeTable, src.nnsTable);
  const auto head = makeTokenNode("namespace") + name;
  const auto decls = foldDecls(node, w, src);
  return head + wrapWithBrace(decls);
}

void
setStructName(XcodeMl::Struct &s, xmlNodePtr node, SourceInfo &src) {
  const auto nameNode = findFirst(node, "name", src.ctxt);
  if (!nameNode || isEmpty(nameNode)) {
    s.setTagName(makeTokenNode(src.getUniqueName()));
    return;
  }
  const auto name = getUnqualIdFromNameNode(nameNode);
  const auto nameSpelling = name->toString(src.typeTable, src.nnsTable);
  s.setTagName(nameSpelling);
}

DEFINE_DECLHANDLER(RecordProc) {
  if (isTrueProp(node, "is_implicit", false)) {
    return CXXCodeGen::makeVoidNode();
  }
  const auto T = src.typeTable.at(getType(node));
  auto structT = llvm::dyn_cast<XcodeMl::Struct>(T.get());
  assert(structT);
  setStructName(*structT, node, src);
  const auto tagName = structT->tagName();

  const auto decls = createNodes(node, "clangDecl", w, src);
  return makeTokenNode("struct") + tagName
      + wrapWithBrace(foldWithSemicolon(decls));
}

DEFINE_DECLHANDLER(TemplateTemplateParmProc) {
  //Temporally impl.
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandTypeTable(src.typeTable, typeTableNode, src.ctxt);
  }
  const auto head = makeTemplateHead(node, w, src);
  return makeTokenNode("template <") + head +  makeTokenNode(">");
}

DEFINE_DECLHANDLER(TemplateTypeParmProc) {
  const auto dtident = getType(node);
  auto T = src.typeTable.at(dtident);
  auto TTPT = llvm::cast<XcodeMl::TemplateTypeParm>(T.get());
  const auto nameSpelling = TTPT->getSpelling().getValue();
  const auto packstr = TTPT->isPack() ? makeTokenNode("...") :
    CXXCodeGen::makeVoidNode();
  return makeTokenNode("typename") + packstr + nameSpelling;
}

DEFINE_DECLHANDLER(TranslationUnitProc) {
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandTypeTable(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsTable(src.nnsTable, nnsTableNode, src.ctxt);
  }
  return foldDecls(node, w, src);
}
DEFINE_DECLHANDLER(TypeAliasTemplateProc){
  if (const auto typeTableNode =
          findFirst(node, "xcodemlTypeTable", src.ctxt)) {
    src.typeTable = expandTypeTable(src.typeTable, typeTableNode, src.ctxt);
  }
  if (const auto nnsTableNode = findFirst(node, "xcodemlNnsTable", src.ctxt)) {
    src.nnsTable = expandNnsTable(src.nnsTable, nnsTableNode, src.ctxt);
  }
  const auto paramNodes =
      findNodes(node, "clangDecl[@class='TemplateTypeParm' or @class='NonTypeTemplateParm']", src.ctxt);
  const auto body = findFirst(node, "clangDecl[@class='TypeAlias']", src.ctxt);

  std::vector<CXXCodeGen::StringTreeRef> params;
  for (auto &&paramNode : paramNodes) {
    params.push_back(w.walk(paramNode, src));
  }

  return makeTokenNode("template") + makeTokenNode("<") + join(",", params)
    + makeTokenNode(">") + w.walk(body, src) ;
}
DEFINE_DECLHANDLER(TypeAliasProc) {

  const auto dtident = getProp(node, "xcodemlTypedefType");
  const auto T = src.typeTable.at(dtident);
  const auto name = getUnqualIdFromIdNode(node, src.ctxt);
  const auto nameSpelling = name->toString(src.typeTable, src.nnsTable);
  return makeTokenNode("using") + nameSpelling + makeTokenNode("=")
      + makeDecl(T, CXXCodeGen::makeVoidNode(), src.typeTable, src.nnsTable);
}

DEFINE_DECLHANDLER(TypedefProc) {
  if (isTrueProp(node, "is_implicit", 0)) {
    return CXXCodeGen::makeVoidNode();
  }
  const auto dtident = getProp(node, "xcodemlTypedefType");
  const auto T = src.typeTable.at(dtident);

  const auto nameNode = findFirst(node, "name", src.ctxt);
  const auto typedefName =
      getUnqualIdFromNameNode(nameNode)->toString(src.typeTable, src.nnsTable);

  return makeTokenNode("typedef")
      + makeDecl(T, typedefName, src.typeTable, src.nnsTable);
}

DEFINE_DECLHANDLER(UsingProc) {
  const auto name =
      getQualifiedName(node, src).toString(src.typeTable, src.nnsTable);
  if (isTrueProp(node, "is_access_declaration", false)) {
    return name;
  }
  return makeTokenNode("using") + name;
}

DEFINE_DECLHANDLER(UsingDirectiveProc) {
  const auto name =
      getQualifiedName(node, src).toString(src.typeTable, src.nnsTable);
  return makeTokenNode("using") + makeTokenNode("namespace") + name;
}

CodeFragment
makeSpecifier(xmlNodePtr node, bool is_in_class_scope) {
  const std::vector<std::tuple<std::string, std::string>> specifiers = {
      std::make_tuple("has_external_storage", "extern"),
      std::make_tuple("is_register", "register"),
      std::make_tuple("is_static_local", "static"),
      std::make_tuple("is_thread_local", "thread_local"),
      std::make_tuple("is_constexpr", "constexpr")
  };
  auto code = CXXCodeGen::makeVoidNode();
  for (auto &&tuple : specifiers) {
    std::string attr, specifier;
    std::tie(attr, specifier) = tuple;
    if (isTrueProp(node, attr.c_str(), false)) {
      code = code + makeTokenNode(specifier);
    }
  }
  if (!is_in_class_scope) {
    return code;
  }
  if (isTrueProp(node, "is_static_data_member", false)) {
    code = code + makeTokenNode("static");
  }
  return code;
}

CodeFragment
emitVarDecl(xmlNodePtr node,
    const CodeBuilder &w,
    SourceInfo &src,
    bool is_in_class_scope) {
  const auto name =
      getQualifiedName(node, src).toString(src.typeTable, src.nnsTable);
  const auto dtident = getProp(node, "xcodemlType");
  const auto T = src.typeTable.at(dtident);
  CodeFragment decl;
  decl = makeSpecifier(node, is_in_class_scope)
      + T->makeDeclaration(name, src.typeTable, src.nnsTable)
    + makeVariableArraySize(node, src, w);

  const auto initializerNode = findFirst(node, "clangStmt", src.ctxt);
  if (!initializerNode) {
    // does not have initalizer: `int x;`
    return decl;
  }
  const auto astClass = getProp(initializerNode, "class");
  if (std::equal(astClass.begin(), astClass.end(), "CXXConstructExpr")) {
    // has initalizer and the variable is of class type
    const auto init = declareClassTypeInit(w, initializerNode, src);
    return wrapWithLangLink(decl + init, node, src);
  }
  const auto init = w.walk(initializerNode, src);
  return decl + makeTokenNode("=") + init;
}

DEFINE_DECLHANDLER(VarProc) {
  return emitVarDecl(node, w, src, false);
}

DEFINE_DECLHANDLER(VarProcInClass) {
  return emitVarDecl(node, w, src, true);
}
DEFINE_DECLHANDLER(NonTypeTemplateParmProc) {
  return emitVarDecl(node, w, src, false);
}

} // namespace

const ClangDeclHandlerType ClangDeclHandlerInClass("class",
    CXXCodeGen::makeInnerNode,
    callCodeBuilder,
    {
        std::make_tuple("ClassTemplate", ClassTemplateProc),
        std::make_tuple(
            "ClassTemplateSpecialization", ClassTemplateSpecializationProc),
        std::make_tuple("ClassTemplatePartialSpecialization",
            ClassTemplatePartialSpecializationProc),
        std::make_tuple("CXXMethod", emitInlineMemberFunction),
        std::make_tuple("CXXConstructor", emitInlineMemberFunction),
        std::make_tuple("CXXConversion", emitInlineMemberFunction),
        std::make_tuple("CXXDestructor", emitInlineMemberFunction),
        std::make_tuple("CXXRecord", CXXRecordProc),
        std::make_tuple("Enum", EnumProc),
        std::make_tuple("FunctionTemplate", FunctionTemplateProc),
        std::make_tuple("EnumConstant", EnumConstantProc),
        std::make_tuple("Field", FieldDeclProc),
        std::make_tuple("Friend", FriendDeclProc),
        std::make_tuple("Using", UsingProc),
        std::make_tuple("TemplateTypeParm", TemplateTypeParmProc),
	std::make_tuple("NonTypeTemplateParm", NonTypeTemplateParmProc),
	std::make_tuple("TemplateTemplateParm", TemplateTemplateParmProc),
        std::make_tuple("TypeAlias", TypeAliasProc),
        std::make_tuple("Typedef", TypedefProc),
	std::make_tuple("TypeAliasTemplate", TypeAliasTemplateProc),
        std::make_tuple("Var", VarProcInClass),
    });

const ClangDeclHandlerType ClangDeclHandler("class",
    CXXCodeGen::makeInnerNode,
    callCodeBuilder,
    {
        std::make_tuple("ClassTemplate", ClassTemplateProc),
        std::make_tuple(
            "ClassTemplateSpecialization", ClassTemplateSpecializationProc),
        std::make_tuple("ClassTemplatePartialSpecialization",
            ClassTemplatePartialSpecializationProc),
        std::make_tuple("CXXConstructor", FunctionProc),
        std::make_tuple("CXXConversion", FunctionProc),
        std::make_tuple("CXXDestructor", FunctionProc),
        std::make_tuple("CXXMethod", FunctionProc),
        std::make_tuple("CXXRecord", CXXRecordProc),
        std::make_tuple("Enum", EnumProc),
        std::make_tuple("EnumConstant", EnumConstantProc),
        std::make_tuple("Field", FieldDeclProc),
        std::make_tuple("Function", FunctionProc),
        std::make_tuple("FunctionTemplate", FunctionTemplateProc),
        std::make_tuple("LinkageSpec", LinkageSpecProc),
        std::make_tuple("Namespace", NamespaceProc),
        std::make_tuple("Record", RecordProc),
        std::make_tuple("TemplateTypeParm", TemplateTypeParmProc),
	std::make_tuple("NonTypeTemplateParm", NonTypeTemplateParmProc),
	std::make_tuple("TemplateTemplateParm", TemplateTemplateParmProc),
        std::make_tuple("TranslationUnit", TranslationUnitProc),
        std::make_tuple("TypeAlias", TypeAliasProc),
	std::make_tuple("TypeAliasTemplate", TypeAliasTemplateProc),
        std::make_tuple("Typedef", TypedefProc),
        std::make_tuple("UsingDirective", UsingDirectiveProc),
        std::make_tuple("Var", VarProc),
    });
