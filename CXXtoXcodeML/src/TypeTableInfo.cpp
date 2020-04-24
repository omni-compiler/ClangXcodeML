#include <libxml/tree.h>
#include "clang/AST/AST.h"
#include "clang/Tooling/Tooling.h"

#include "CXXtoXML.h"
#include "TypeTableInfo.h"
#include "NnsTableInfo.h"
// #include "DeclarationsVisitor.h"
#include "ClangOperator.h"
#include "XcodeMlNameElem.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <cctype>

using namespace clang;
using namespace llvm;


static cl::opt<bool> OptTraceTypeTable("trace-typeTable",
    cl::desc("emit traces on <typeTable>"),
    cl::cat(CXX2XMLCategory));

static cl::opt<bool> OptFullTraceTypeTable("fulltrace-typeTable",
    cl::desc("emit full-traces on <typeTable>"),
    cl::cat(CXX2XMLCategory));

static cl::opt<bool> OptDisableTypeTable("disable-typeTable",
    cl::desc("disable <typeTable>"),
    cl::cat(CXX2XMLCategory));

static cl::opt<bool> OptIgnoreUnknownType("ignore-unknown-type",
    cl::desc("ignore unknown type"),
    cl::cat(CXX2XMLCategory));

static cl::opt<std::string> OptTypeNameMap("typenamemap",
    cl::desc("a map file of typename substitution"),
    cl::cat(CXX2XMLCategory));

static std::ifstream mapfile;
static bool map_is_already_set = false;
static std::map<std::string, std::string> typenamemap;

// constructor
TypeTableInfo::TypeTableInfo(
    MangleContext *MC, InheritanceInfo *II, NnsTableInfo *NTI)
    : mangleContext(MC), inheritanceinfo(II), nnstableinfo(NTI) {
  mapFromNameToQualType.clear();
  mapFromQualTypeToName.clear();
  mapFromQualTypeToXmlNodePtr.clear();
  seqForBasicType = 0;
  seqForPointerType = 0;
  seqForFunctionType = 0;
  seqForArrayType = 0;
  seqForStructType = 0;
  seqForUnionType = 0;
  seqForEnumType = 0;
  seqForClassType = 0;
  seqForTemplateTypeParmType = 0;
  seqForInjectedClassNameType = 0;
  seqForMemberPointerType = 0;
  seqForTemplateSpecializationType = 0;
  seqForDependentNameType = 0;
  seqForOtherType = 0;

  TypeElements.clear();
  useLabelType = false;
}

std::string
TypeTableInfo::registerBasicType(QualType T) {
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Basic" << seqForBasicType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerTemplateSpecializationType(QualType T) {
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "TemplateSpecialization" << seqForTemplateSpecializationType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerPointerType(QualType T) {
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());
  using clang::Type;

  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::Pointer:
  case Type::BlockPointer:
  case Type::LValueReference:
  case Type::RValueReference: OS << "Pointer" << seqForPointerType++; break;
  default: abort();
  }
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerFunctionType(QualType T) {
  using clang::Type;
  assert(T->getTypeClass() == Type::FunctionProto
      || T->getTypeClass() == Type::FunctionNoProto);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Function" << seqForFunctionType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerArrayType(QualType T) {
  std::string name = mapFromQualTypeToName[T];
  using clang::Type;
  assert(name.empty());

  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::ConstantArray: OS << "ConstantArray" << seqForArrayType++; break;
  case Type::IncompleteArray:
    OS << "ImcompleteArray" << seqForArrayType++;
    break;
  case Type::VariableArray: OS << "VariableArray" << seqForArrayType++; break;
  case Type::DependentSizedArray:
    OS << "DependentSizeArray" << seqForArrayType++;
    break;
  default: abort();
  }
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerRecordType(QualType T) {
  assert(T->getTypeClass() == clang::Type::Record);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  if (T->getAsCXXRecordDecl()) {
    OS << "Class" << seqForStructType++;
    // XXX: temporary implementation
  } else if (T->isStructureType()) {
    OS << "Struct" << seqForStructType++;
  } else if (T->isUnionType()) {
    OS << "Union" << seqForUnionType++;
  } else {
    abort();
  }
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerEnumType(QualType T) {
  assert(T->getTypeClass() == clang::Type::Enum);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Enum" << seqForEnumType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerTemplateTypeParmType(QualType T) {
  assert(T->getTypeClass() == clang::Type::TemplateTypeParm);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "TemplateTypeParm" << seqForTemplateTypeParmType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerInjectedClassNameType(QualType T) {
  assert(T->getTypeClass() == clang::Type::InjectedClassName);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "InjectedClassName" << seqForInjectedClassNameType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string
TypeTableInfo::registerMemberPointerType(QualType T) {
  assert(T->getTypeClass() == clang::Type::MemberPointer);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "MemberPointer" << seqForMemberPointerType++;
  return mapFromQualTypeToName[T] = OS.str();
}
std::string
TypeTableInfo::registerDependentNameType(QualType T){
  assert(T->getTypeClass() == clang::Type::DependentName);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "DependentName" << seqForDependentNameType++;
  return mapFromQualTypeToName[T] = OS.str();
}
std::string
TypeTableInfo::registerOtherType(QualType T) {
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Other" << seqForOtherType++;
  return mapFromQualTypeToName[T] = OS.str();
}

xmlNodePtr
TypeTableInfo::createNode(QualType T, const char *fieldname, xmlNodePtr) {
  xmlNodePtr N = xmlNewNode(nullptr, BAD_CAST fieldname);
  xmlNewProp(N, BAD_CAST "type", BAD_CAST getTypeName(T).c_str());
  return N;
}

void
TypeTableInfo::pushType(const QualType &T, xmlNodePtr node) {
  std::get<1>(typeTableStack.top()).push_back(T);
  TypeElements[T] = node;
}

static const char *
getTagKindAsString(clang::TagTypeKind ttk) {
  switch (ttk) {
  case TTK_Struct: return "struct";
  case TTK_Union: return "union";
  case TTK_Class: return "class";
  case TTK_Enum: return "enum";
  case TTK_Interface: return "__interface__";
  }
  return "NULL";
}

static xmlNodePtr
makeSymbolsNodeForCXXRecordDecl(TypeTableInfo &TTI, const CXXRecordDecl *RD) {
  assert(RD);
  auto symbolsNode = xmlNewNode(nullptr, BAD_CAST "symbols");
  if (!RD->hasDefinition()) {
    return symbolsNode; // empty <symbols>
  }
  auto def = RD->getDefinition();

  // data members
  for (auto &&field : def->fields()) {
    auto idNode = makeIdNodeForFieldDecl(TTI, field);
    xmlAddChild(symbolsNode, idNode);
  }

  // static or non-static member functions
  for (auto &&method : def->methods()) {
    auto idNode = makeIdNodeForCXXMethodDecl(TTI, method);
    xmlAddChild(symbolsNode, idNode);
  }

  return symbolsNode;
}

static xmlNodePtr
makeSymbolsNodeForRecordType(TypeTableInfo &TTI, const RecordType *RT) {
  assert(RT);
  if (auto CRD = RT->getAsCXXRecordDecl()) {
    return makeSymbolsNodeForCXXRecordDecl(TTI, CRD);
  }
  auto symbolsNode = xmlNewNode(nullptr, BAD_CAST "symbols");
  auto RD = RT->getDecl();
  if (!RD) {
    return symbolsNode; // empty <symbols>
  }
  auto def = RD->getDefinition();
  if (!def) {
    return symbolsNode; // empty <symbols>
  }
  auto fields = def->fields();
  for (auto field : fields) {
    auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
    xmlNewProp(idNode,
        BAD_CAST "type",
        BAD_CAST TTI.getTypeName(field->getType()).c_str());
    const auto fieldName = field->getIdentifier();
    if (fieldName) {
      /* Emit only if the field has name.
       * Some field does not have name.
       *  Example: `struct A { int : 0; }; // unnamed bit field`
       */
      auto nameNode = makeNameNode(TTI, field);
      xmlAddChild(idNode, nameNode);
    }
    xmlAddChild(symbolsNode, idNode);
  }
  return symbolsNode;
}

static xmlNodePtr
makeInheritanceNode(TypeTableInfo &TTI, const CXXRecordDecl *RD) {
  assert(RD);
  auto inheritanceNode = xmlNewNode(nullptr, BAD_CAST "inheritedFrom");
  if (!RD->hasDefinition()) {
    return inheritanceNode; // empty node
  }
  const auto def = RD->getDefinition();

  for (auto &&base : def->bases()) {
    auto typeNode = xmlNewNode(nullptr, BAD_CAST "typeName");
    xmlNewProp(typeNode,
        BAD_CAST "ref",
        BAD_CAST TTI.getTypeName(base.getType()).c_str());
    xmlNewProp(typeNode,
        BAD_CAST "access",
        BAD_CAST AccessSpec(base.getAccessSpecifier()).c_str());
    xmlNewProp(typeNode,
        BAD_CAST "is_virtual",
        BAD_CAST(base.isVirtual() ? "1" : "0"));
    xmlAddChild(inheritanceNode, typeNode);
  }
  return inheritanceNode;
}

namespace {

void
commonSetUpForRecordDecl(
    xmlNodePtr node, const RecordDecl *RD, TypeTableInfo &TTI) {
  assert(RD);
  xmlNewProp(node,
      BAD_CAST "cxx_class_kind",
      BAD_CAST getTagKindAsString(RD->getTagKind()));
  if (RD->isAnonymousStructOrUnion())
    xmlNewProp(node, BAD_CAST "is_anonymous", BAD_CAST "1");

  const auto className = RD->getName();
  xmlNewChild(node, nullptr, BAD_CAST "name", BAD_CAST className.data());

  if (const auto CTS = dyn_cast<ClassTemplateSpecializationDecl>(RD)) {
    xmlNewProp(node, BAD_CAST "is_template_instantiation", BAD_CAST "1");
    auto ST = CTS->getSpecializedTemplate();
    const auto templArgs = xmlNewNode(nullptr, BAD_CAST "templateArguments");
    for (auto &&arg : CTS->getTemplateArgs().asArray()) {
      switch (arg.getKind()) {
      case TemplateArgument::Null:
        xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "null"));
        break;
      case TemplateArgument::Type: {
        const auto typeNode = xmlNewNode(nullptr, BAD_CAST "typeName");
        xmlNewProp(typeNode,
                   BAD_CAST "ref",
                   BAD_CAST TTI.getTypeName(arg.getAsType()).c_str());
        xmlAddChild(templArgs, typeNode);
        break;
      }
      case TemplateArgument::Declaration:
        xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "declaration"));
        break;
      case TemplateArgument::NullPtr:
        xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "nullptr"));
        break;
      case TemplateArgument::Integral:
        {
        const auto integNode = xmlNewNode(nullptr, BAD_CAST "integral");
        xmlNewProp(integNode,
                   BAD_CAST "value",
                   BAD_CAST arg.getAsIntegral().toString(10).c_str());
        xmlAddChild(templArgs, integNode);
        }
        break;
      case TemplateArgument::Template:{
        auto tn = arg.getAsTemplate();
        xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "template"));
        break;
      }
      case TemplateArgument::TemplateExpansion:
        xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "template_expansion"));
        break;
      case TemplateArgument::Expression:{
        const auto exprNode = xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "expression"));
        const auto theExpr = arg.getAsExpr();
      }
        break;
      case TemplateArgument::Pack:
        xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "pack"));
        break;
      }
    }
    xmlAddChild(node, templArgs);
  }
  if (const auto CRD = dyn_cast<CXXRecordDecl>(RD)) {
    xmlAddChild(node, makeInheritanceNode(TTI, CRD));
  }
}

} // namespace

void
TypeTableInfo::registerType(QualType T, xmlNodePtr *retNode, xmlNodePtr) {
  bool isQualified = false;
  xmlNodePtr Node = nullptr;
  std::string rawname;
  using clang::Type;

  if (T.isNull()) {
    return;
  };

  if (!T.isCanonical()) {
    registerType(T.getCanonicalType(), retNode, nullptr);
    mapFromQualTypeToName[T] = mapFromQualTypeToName[T.getCanonicalType()];
    return;
  }

  if (mapFromQualTypeToName.find(T) != mapFromQualTypeToName.end()) {
    if (retNode != nullptr) {
      *retNode = mapFromQualTypeToXmlNodePtr[T];
    }
    return;
  }

  if (!isa<const clang::ArrayType>(T.getTypePtr())) {
    if (T.isConstQualified()) {
      isQualified = true;
    }
    if (T.isVolatileQualified()) {
      isQualified = true;
    }
    if (T.isRestrictQualified()) {
      isQualified = true;
    }
  }

  if (isQualified) {
    rawname = registerBasicType(T);
    // XXX: temporary imcompletearray
    Node = createNode(T, "basicType", nullptr);

    xmlNewProp(Node,
        BAD_CAST "name",
        BAD_CAST getTypeName(T.getUnqualifiedType()).c_str());

    if (T.isConstQualified()) {
      xmlNewProp(Node, BAD_CAST "is_const", BAD_CAST "1");
    }
    if (T.isVolatileQualified()) {
      xmlNewProp(Node, BAD_CAST "is_volatile", BAD_CAST "1");
    }
    if (T.isRestrictQualified()) {
      xmlNewProp(Node, BAD_CAST "is_restrict", BAD_CAST "1");
    }

    pushType(T, Node);
  } else
    switch (T->getTypeClass()) {
    case Type::Builtin: {
      const Type *Tptr = T.getTypePtrOrNull();
      ASTContext &CXT = mangleContext->getASTContext();
      PrintingPolicy PP(CXT.getLangOpts());

      // XXX: temporary implementation
      rawname = static_cast<const BuiltinType *>(Tptr)->getName(PP).str();
      for (auto I = rawname.begin(); I != rawname.end(); ++I) {
        if (!std::isalnum(*I)) {
          *I = '_';
        }
      }
      mapFromQualTypeToName[T] = rawname;
      break;
    }

    case Type::Complex:
      rawname = registerOtherType(T);
      // XXX: temporary implementation
      if (isQualified) {
        Node = createNode(T, "basicType", nullptr);
      } else {
        Node = createNode(T, "complexType", nullptr);
      }
      pushType(T, Node);
      break;

    case Type::Pointer:
    case Type::BlockPointer: {
      rawname = registerPointerType(T);
      Node = createNode(T, "pointerType", nullptr);
      if (const auto *PT =
            dyn_cast<const clang::PointerType>(T.getTypePtr())) {
        registerType(PT->getPointeeType(), nullptr, nullptr);
        xmlNewProp(Node,
            BAD_CAST "ref",
            BAD_CAST getTypeName(PT->getPointeeType()).c_str());
      }
      pushType(T, Node);
    } break;
    case Type::MemberPointer: {
      rawname = registerMemberPointerType(T);
      Node = createNode(T, "memberPointerType", nullptr);
      const auto MPT = T.getTypePtr()->getAs<MemberPointerType>();
      assert(MPT);
      registerType(MPT->getPointeeType(), nullptr, nullptr);
      xmlNewProp(Node,
          BAD_CAST "ref",
          BAD_CAST getTypeName(MPT->getPointeeType()).c_str());
      const auto parent = QualType(MPT->getClass(), 0);
      registerType(parent, nullptr, nullptr);
      xmlNewProp(
          Node, BAD_CAST "record", BAD_CAST getTypeName(parent).c_str());
      pushType(T, Node);
      break;
    }
    case Type::LValueReference:
    case Type::RValueReference: {
      rawname = registerPointerType(T);
      Node = createNode(T, "pointerType", nullptr);
      xmlNewProp(Node,
          BAD_CAST "reference",
          T->getTypeClass() == Type::LValueReference ? BAD_CAST "lvalue"
                                                     : BAD_CAST "rvalue");
      if (const auto RT = dyn_cast<ReferenceType>(T.getTypePtr())) {
        const auto Pointee = RT->getPointeeType();
        registerType(Pointee, nullptr, nullptr);
        xmlNewProp(
            Node, BAD_CAST "ref", BAD_CAST getTypeName(Pointee).c_str());
      }
      pushType(T, Node);
    } break;

    case Type::IncompleteArray:
    case Type::VariableArray:
    case Type::DependentSizedArray:
    case Type::ConstantArray: {
      ASTContext &CXT = mangleContext->getASTContext();
      const clang::ArrayType *AT = CXT.getAsArrayType(T);
      if (AT) {
        registerType(AT->getElementType(), nullptr, nullptr);
      }
      rawname = registerArrayType(T);
      Node = createNode(T, "arrayType", nullptr);
      if (AT) {
        xmlNewProp(Node,
            BAD_CAST "element_type",
            BAD_CAST getTypeName(AT->getElementType()).c_str());
        const ConstantArrayType *CAT = dyn_cast<const ConstantArrayType>(AT);
        if (CAT)
          xmlNewProp(Node,
                     BAD_CAST "array_size",
                     BAD_CAST CAT->getSize().toString(10, false).c_str());
      }
      pushType(T, Node);
    } break;

    case Type::DependentSizedExtVector:
    case Type::Vector:
    case Type::ExtVector:
      rawname = registerOtherType(T);
      // XXX: temporary implementation
      Node = createNode(T, "vectorType", nullptr);
      pushType(T, Node);
      break;

    case Type::FunctionProto:
    case Type::FunctionNoProto: {
      const auto *FT = dyn_cast<const clang::FunctionType>(T.getTypePtr());
      if (FT) {
        registerType(FT->getReturnType(), nullptr, nullptr);
        if(mapFromQualTypeToName.find(T) != mapFromQualTypeToName.end())
            break;
      }
      rawname = registerFunctionType(T);
      Node = createNode(T, "functionType", nullptr);
      if (FT) {
        xmlNewProp(Node,
            BAD_CAST "return_type",
            BAD_CAST getTypeName(FT->getReturnType()).c_str());
        xmlNewProp(
            Node, BAD_CAST "is_const", BAD_CAST(FT->isConst() ? "1" : "0"));
        xmlNewProp(Node,
            BAD_CAST "is_volatile",
            BAD_CAST(FT->isVolatile() ? "1" : "0"));
        xmlNewProp(Node,
            BAD_CAST "is_restrict",
            BAD_CAST(FT->isRestrict() ? "1" : "0"));
      }
      if (auto FTP = dyn_cast<FunctionProtoType>(FT)) {
        auto paramsNode = xmlNewNode(nullptr, BAD_CAST "params");
        for (auto &paramT : FTP->getParamTypes()) {
          auto paramNode = xmlNewNode(nullptr, BAD_CAST "paramTypeName");
          xmlNewProp(paramNode,
              BAD_CAST "type",
              BAD_CAST getTypeName(paramT).c_str());
          xmlAddChild(paramsNode, paramNode);
        }
        if (FTP->isVariadic()) {
          auto ellipNode = xmlNewNode(nullptr, BAD_CAST "ellipsis");
          xmlAddChild(paramsNode, ellipNode);
        }
        xmlAddChild(Node, paramsNode);
      }
      pushType(T, Node);
    } break;

    case Type::UnresolvedUsing:
    case Type::Paren:
    case Type::Typedef:
      rawname = registerOtherType(T);
      // XXX: temporary implementation
      Node = createNode(T, "typedefType", nullptr);
      pushType(T, Node);
      break;
    case Type::Decltype:{
      rawname = registerOtherType(T);
      Node = createNode(T, "decltypeType", nullptr);
      auto DT = dyn_cast<DecltypeType>(T);
      //auto UT = DT->getUnderlyingType();
      //auto E = DT->getUnderlyingExpr();
      //UT->dump();
      //E->dump();
      pushType(T,Node);
      break;
    }
    case Type::UnaryTransform:{
      rawname = registerOtherType(T);
      auto UT = dyn_cast<UnaryTransformType>(T);
      Node = createNode(T, "unaryTransformType", nullptr);
      xmlNewProp(Node, BAD_CAST "Typeparam",
                 BAD_CAST (getTypeName(UT->getBaseType()).c_str()));
      pushType(T, Node);
      break;
    }
    case Type::Adjusted:
    case Type::Decayed:
    case Type::TypeOfExpr:
    case Type::TypeOf:

      rawname = registerOtherType(T);
      // XXX: temporary implementation
      Node = createNode(T, "otherType", nullptr);
      xmlNewProp(
          Node, BAD_CAST "clang_type_class", BAD_CAST(T->getTypeClassName()));
      pushType(T, Node);
      break;

    case Type::Record: {
      rawname = registerRecordType(T);
      if (auto RD = T->getAsCXXRecordDecl()) {
        Node = createNode(T, "classType", nullptr);
        commonSetUpForRecordDecl(Node, RD, *this);
        const auto DC = RD->getDeclContext();
        assert(DC);
        const auto nns = nnstableinfo->getNnsName(DC);
        xmlNewProp(Node, BAD_CAST "nns", BAD_CAST(nns.c_str()));
        pushType(T, Node);
      } else if (T->isStructureType()) {
        Node = createNode(T, "structType", nullptr);
        pushType(T, Node);
      } else if (T->isUnionType()) {
        Node = createNode(T, "unionType", nullptr);
        pushType(T, Node);
      } else {
        // XXX: temporary implementation
        Node = createNode(T, "unknownRecordType", nullptr);
        pushType(T, Node);
      }
      xmlAddChild(Node,
          makeSymbolsNodeForRecordType(*this, llvm::cast<RecordType>(T)));
      // T must be of clang::RecordType so able to be casted
      break;
    }

    case Type::Enum: {
      rawname = registerEnumType(T);
      Node = createNode(T, "enumType", nullptr);
      auto ED = llvm::cast<EnumType>(T)->getDecl();
      if (!ED) {
        break;
      }
      const auto nameNode = makeNameNode(*this, ED);
      xmlAddChild(Node, nameNode);
      auto def = ED->getDefinition();
      if (!def) {
        /* Forward declaration of enum is available in C++11
         *  Example : `enum E : unsigned int;`
         */
        break;
      }
      auto symbolsNode = xmlNewNode(nullptr, BAD_CAST "symbols");
      auto names = def->enumerators();
      for (auto name : names) {
        const auto constantName = name->getIdentifier();
        assert(constantName);
        auto idNode = xmlNewNode(nullptr, BAD_CAST "id");
        xmlNewChild(idNode,
            nullptr,
            BAD_CAST "name",
            BAD_CAST constantName->getName().data());
        xmlAddChild(symbolsNode, idNode);
      }
      xmlAddChild(Node, symbolsNode);
      pushType(T, Node);
      break;
    }

    case Type::TemplateTypeParm: {
      rawname = registerTemplateTypeParmType(T);
      Node = createNode(T, "TemplateTypeParmType", nullptr);
      const auto TTP = cast<TemplateTypeParmType>(T.getTypePtr());
      xmlNewProp(Node,
          BAD_CAST "clang_depth",
          BAD_CAST std::to_string(TTP->getDepth()).c_str());
      xmlNewProp(Node,
          BAD_CAST "clang_index",
          BAD_CAST std::to_string(TTP->getIndex()).c_str());
      xmlNewProp(Node, BAD_CAST "pack", (TTP->isParameterPack() ?
                                         BAD_CAST "1" : BAD_CAST "0"));
      const auto nameNode = makeNameNode(*this, TTP);
      xmlAddChild(Node, nameNode);
      pushType(T, Node);
      break;
    }

    case Type::InjectedClassName: {
      // The injected class name of a class template
      // or class template partial specialization
      rawname = registerInjectedClassNameType(T);
      Node = createNode(T, "injectedClassNameType", nullptr);
      const auto ICN = cast<InjectedClassNameType>(T);
      if (const auto RD = ICN->getDecl()) {
        commonSetUpForRecordDecl(Node, RD, *this);
      }
      pushType(T, Node);
      break;
    }
    case Type::TemplateSpecialization:{
      rawname = registerTemplateSpecializationType(T);
      //std::cerr << "OTHER TYPE1"<< T->getTypeClassName()<<std::endl;
      // XXX: temporary implementation
      Node = createNode(T, "TemplateSpecializationType", nullptr);
      xmlNewProp(
                 Node, BAD_CAST "clang_type_class", BAD_CAST(T->getTypeClassName()));
      auto TST = cast<TemplateSpecializationType>(T);
      auto TD = TST->getTemplateName().getUnderlying().getAsTemplateDecl();

      //TST->dump();
      xmlNewChild(Node,
                  nullptr,
                  BAD_CAST "name",
                  BAD_CAST TD->getName().data());
      const auto templArgs = xmlNewNode(nullptr, BAD_CAST "templateArguments");
      for(auto arg : *TST){
        switch (arg.getKind()) {
        case TemplateArgument::Null:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "null"));
          break;
        case TemplateArgument::Type: {
          const auto typeNode = xmlNewNode(nullptr, BAD_CAST "typeName");
          xmlNewProp(typeNode,
                     BAD_CAST "ref",
                     BAD_CAST getTypeName(arg.getAsType()).c_str());
          xmlAddChild(templArgs, typeNode);
          break;
        }
        case TemplateArgument::Declaration:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "declaration"));
          break;
        case TemplateArgument::NullPtr:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "nullptr"));
          break;
        case TemplateArgument::Integral:
          {
            const auto integNode = xmlNewNode(nullptr, BAD_CAST "integral");
            xmlNewProp(integNode,
                       BAD_CAST "value",
                       BAD_CAST arg.getAsIntegral().toString(10).c_str());
            xmlAddChild(templArgs, integNode);
          }
          break;
        case TemplateArgument::Template:{
          const auto tnode = xmlNewNode(nullptr, BAD_CAST "template");
          auto tn = arg.getAsTemplate();
          auto tnt = tn.getAsTemplateDecl();
          //tnt->dump();
          //std::cout <<getTypeName(tn) <<std:endl;
          if(tnt != nullptr){
            xmlNewProp(tnode,
                       BAD_CAST "name",
                       BAD_CAST tnt->getName().str().c_str());
          }
          xmlAddChild(templArgs, tnode);
        }
          break;
        case TemplateArgument::TemplateExpansion:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "template_expansion"));
          break;
        case TemplateArgument::Expression:{
          
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "expression"));
          break;
        }
        case TemplateArgument::Pack:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "pack"));
          break;
        }
      }
      xmlAddChild(Node, templArgs);
      pushType(T, Node);
      break;
    }
    case Type::DependentName:{
      rawname = registerDependentNameType(T);
      auto DN =cast<DependentNameType>(T);
      Node = createNode(T, "DependentNameType", nullptr);

      auto II = DN->getIdentifier();
      auto NS = DN->getQualifier();
      StringRef SR;
      if(NS->getAsNamespace()){
      }
      if(NS->getAsNamespaceAlias()){
      }
      if(NS->getAsRecordDecl()){
      }
      if(auto TT=NS->getAsType()){
        auto QT = QualType(TT, 0);
        xmlNewProp( Node, BAD_CAST "dependtype",
                    BAD_CAST (getTypeName(QT).c_str()));
      }
      xmlNewProp(
                 Node, BAD_CAST "symbol", BAD_CAST(II->getName()).str().c_str());
      pushType(T, Node);
      break;
    }
    case Type::PackExpansion:{
      rawname = registerOtherType(T);
      auto PET = dyn_cast<PackExpansionType>(T);
      Node = createNode(T, "PackExpansionType", nullptr);
      xmlNewProp (Node,
                  BAD_CAST "pattern",
                  BAD_CAST(getTypeName(PET->getPattern()).c_str()));
      pushType(T, Node);
      break;
    }
    case Type::DependentTemplateSpecialization:{
      rawname = registerOtherType(T);
      auto DTS = dyn_cast<DependentTemplateSpecializationType>(T);
      //DTS->dump();

      // XXX: temporary implementation
      Node = createNode(T, "dependentTemplateSpecializationType", nullptr);
      const auto templArgs = xmlNewNode(nullptr, BAD_CAST "templateArguments");
      for(auto arg : *DTS){
        switch (arg.getKind()) {
        case TemplateArgument::Null:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "null"));
          break;
        case TemplateArgument::Type: {
          const auto typeNode = xmlNewNode(nullptr, BAD_CAST "typeName");
          xmlNewProp(typeNode,
                     BAD_CAST "ref",
                     BAD_CAST getTypeName(arg.getAsType()).c_str());
          xmlAddChild(templArgs, typeNode);
          break;
        }
        case TemplateArgument::Declaration:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "declaration"));
          break;
        case TemplateArgument::NullPtr:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "nullptr"));
          break;
        case TemplateArgument::Integral:
          {
            const auto integNode = xmlNewNode(nullptr, BAD_CAST "integral");
            xmlNewProp(integNode,
                       BAD_CAST "value",
                       BAD_CAST arg.getAsIntegral().toString(10).c_str());
            xmlAddChild(templArgs, integNode);
          }
          break;
        case TemplateArgument::Template:{
          const auto tnode = xmlNewNode(nullptr, BAD_CAST "template");
          auto tn = arg.getAsTemplate();
          auto tnt = tn.getAsTemplateDecl();
          //tnt->dump();
          //std::cout <<getTypeName(tn) <<std:endl;
          if(tnt != nullptr){
            xmlNewProp(tnode,
                       BAD_CAST "name",
                       BAD_CAST tnt->getName().str().c_str());
          }
          xmlAddChild(templArgs, Node);
        }
          break;
        case TemplateArgument::TemplateExpansion:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "template_expansion"));
          break;
        case TemplateArgument::Expression:{
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "expression"));
          break;
        }
        case TemplateArgument::Pack:
          xmlAddChild(templArgs, xmlNewNode(nullptr, BAD_CAST "pack"));
          break;
        }
      }
      pushType(T, Node);
      break;
    }
    case Type::Atomic:{
      rawname = registerOtherType(T);
      auto AT = dyn_cast<AtomicType>(T);
      Node = createNode(T, "atomicType", nullptr);
      xmlNewProp( Node, BAD_CAST "valueType",
                  BAD_CAST(getTypeName(AT->getValueType()).c_str()));
      pushType(T, Node);
      break;
    }
    case Type::Elaborated:
    case Type::Attributed:
    case Type::SubstTemplateTypeParm:
    case Type::SubstTemplateTypeParmPack:
    case Type::Auto:
    case Type::ObjCObject:
    case Type::ObjCInterface:
    case Type::ObjCObjectPointer:

      rawname = registerOtherType(T);
      // XXX: temporary implementation
      Node = createNode(T, "otherType", nullptr);
      xmlNewProp(
          Node, BAD_CAST "clang_type_class", BAD_CAST(T->getTypeClassName()));
      pushType(T, Node);
      break;
    default:
      T->dump();
      abort();
    }

  if (retNode != nullptr) {
    *retNode = Node;
  }
  mapFromNameToQualType[rawname] = T;
  mapFromQualTypeToXmlNodePtr[T] = Node;
}

void
TypeTableInfo::registerLabelType(void) {
  useLabelType = true;
}

std::string
TypeTableInfo::getTypeName(QualType T) {
  if (T.isNull()) {
    return "nullType";
  };

  const auto iter = mapFromQualTypeToName.find(T);
  std::string name;
  if (iter != mapFromQualTypeToName.end()) {
    name = iter->second;
  } else {
    registerType(T, nullptr, nullptr);
    name = mapFromQualTypeToName[T];
  }

  if (!map_is_already_set) {
    typenamemap["unsigned_int"] = "unsigned";
  }
  if (!OptTypeNameMap.empty()) {
    if (!map_is_already_set) {
      std::cerr << "use " << OptTypeNameMap << " as a typenamemap file"
                << std::endl;
      mapfile.open(OptTypeNameMap);
      if (mapfile.fail()) {
        std::cerr << OptTypeNameMap << ": cannot open" << std::endl;
        exit(1);
      }
      map_is_already_set = true;

      typenamemap.clear();
      std::string line;
      while (std::getline(mapfile, line)) {
        std::istringstream iss(line);
        std::string lhs, rhs;
        iss >> lhs >> rhs;
        if (!iss) {
          std::cerr << OptTypeNameMap << ": read error" << std::endl;
          exit(1);
        }
        typenamemap[lhs] = rhs;
        // std::cerr << "typenamemap: " << lhs << "->" << rhs << std::endl;
      }
    }
    std::string rhs = typenamemap[name];
    return !rhs.empty() ? rhs : name;
  }

  map_is_already_set = true;
  return name;
}

std::string
TypeTableInfo::getTypeNameForLabel(void) {
  if (!typenamemap["Label"].empty()) {
    return typenamemap["Label"];
  } else {
    return "Label";
  }
}

std::vector<BaseClass>
TypeTableInfo::getBaseClasses(clang::QualType type) {
  return inheritanceinfo->getInheritance(type);
}

void
TypeTableInfo::addInheritance(clang::QualType derived, BaseClass base) {
  inheritanceinfo->addInheritance(derived, base);
}

bool
TypeTableInfo::hasBaseClass(clang::QualType type) {
  return !(inheritanceinfo->getInheritance(type).empty());
}

void
TypeTableInfo::setNormalizability(clang::QualType T, bool b) {
  normalizability[T] = b;
}

bool TypeTableInfo::isNormalizable(clang::QualType) {
  // return normalizability[T];
  return false;
}

void
TypeTableInfo::pushTypeTableStack(xmlNodePtr typeTableNode) {
  typeTableStack.push(std::make_tuple(typeTableNode, std::vector<QualType>()));
}

void
TypeTableInfo::popTypeTableStack() {
  assert(!typeTableStack.empty());
  const auto typeTableNode = std::get<0>(typeTableStack.top());
  const auto latestTypes = std::get<1>(typeTableStack.top());
  for (auto T : latestTypes) {
    if (TypeElements.find(T) != TypeElements.end()) {
      /*
       * If data type definition element exists,
       * add it to current typeTable.
       * Fundamental types do not have data type definition elements.
       * (Example: `int`)
       */
      xmlAddChild(typeTableNode, TypeElements[T]);
      TypeElements.erase(T);
    }
    const auto name = mapFromQualTypeToName.at(T);
    mapFromNameToQualType.erase(name);
    mapFromQualTypeToName.erase(T);
  }
  typeTableStack.pop();
}

void
TypeTableInfo::dump() {
  for (auto &pair : mapFromNameToQualType) {
    auto name = pair.first;
    auto type = pair.second;
    std::cerr << name << ": " << type.getAsString() << std::endl;
  }
}

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
