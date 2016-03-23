#include "XcodeMlVisitorBase.h"
#include "SymbolsVisitor.h"
#include "TypeTableVisitor.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <cctype>

using namespace clang;
using namespace llvm;

static cl::opt<bool>
OptTraceTypeTable("trace-typeTable",
                  cl::desc("emit traces on <typeTable>"),
                  cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptFullTraceTypeTable("fulltrace-typeTable",
                      cl::desc("emit full-traces on <typeTable>"),
                      cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableTypeTable("disable-typeTable",
                    cl::desc("disable <typeTable>"),
                    cl::cat(C2XcodeMLCategory));

static cl::opt<std::string>
OptTypeNameMap("typenamemap",
               cl::desc("a map file of typename substitution"),
               cl::cat(C2XcodeMLCategory));

static std::ifstream mapfile;
static bool map_is_already_set = false;
static std::map<std::string, std::string> typenamemap;

static std::string
make_comment(Decl *decl, std::string message) {
  std::string comment("PreVisitDecl::");
  comment += decl->getDeclKindName();
  comment += ": ";
  comment += message;
  return comment;
}

bool
TypeTableVisitor::FullTrace(void) const {
  return OptFullTraceTypeTable;
}

TypeTableInfo::TypeTableInfo(MangleContext *MC, InheritanceInfo *II) :
  mangleContext(MC),
  inheritanceinfo(II)
{
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
  seqForOtherType = 0;

  basicTypeNodes.clear();
  pointerTypeNodes.clear();
  functionTypeNodes.clear();
  arrayTypeNodes.clear();
  structTypeNodes.clear();
  unionTypeNodes.clear();
  enumTypeNodes.clear();
  classTypeNodes.clear();
  otherTypeNodes.clear();

  useLabelType = false;
}

std::string TypeTableInfo::registerBasicType(QualType T){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Basic" << seqForBasicType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerPointerType(QualType T){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::Pointer:
  case Type::BlockPointer:
  case Type::LValueReference:
  case Type::RValueReference:
  case Type::MemberPointer:
    OS << "Pointer" << seqForPointerType++;
    break;
  default:
    abort();
  }
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerFunctionType(QualType T){
  assert(T->getTypeClass() == Type::FunctionProto
         || T->getTypeClass() == Type::FunctionNoProto);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Function" << seqForFunctionType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerArrayType(QualType T){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::ConstantArray:
    OS << "ConstantArray" << seqForArrayType++;
    break;
  case Type::IncompleteArray:
    OS << "ImcompleteArray" << seqForArrayType++;
    break;
  case Type::VariableArray:
    OS << "VariableArray" << seqForArrayType++;
    break;
  case Type::DependentSizedArray:
    OS << "DependentSizeArray" << seqForArrayType++;
    break;
  default:
    abort();
  }
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerRecordType(QualType T){
  assert(T->getTypeClass() == Type::Record);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  if (T->isStructureType()) {
    OS << "Struct" << seqForStructType++;
  } else if (T->isUnionType()) {
    OS << "Union" << seqForUnionType++;
  } else if (T->isClassType()) {
    OS << "Class" << seqForClassType++;
  } else {
    abort();
  }
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerEnumType(QualType T){
  assert(T->getTypeClass() == Type::Enum);
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Enum" << seqForEnumType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerOtherType(QualType T){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "Other" << seqForOtherType++;
  return mapFromQualTypeToName[T] = OS.str();
}

xmlNodePtr TypeTableInfo::createNode(QualType T, const char *fieldname, xmlNodePtr traversingNode) {
  xmlNodePtr N = xmlNewNode(nullptr, BAD_CAST fieldname);
  bool isQualified = false;
  xmlNewProp(N, BAD_CAST "type", BAD_CAST getTypeName(T).c_str());

  if (T.isConstQualified()) {
    xmlNewProp(N, BAD_CAST "is_const", BAD_CAST "1");
    isQualified = true;
  }
  if (T.isVolatileQualified()) {
    xmlNewProp(N, BAD_CAST "is_volatile", BAD_CAST "1");
    isQualified = true;
  }
  if (T.isRestrictQualified()) {
    xmlNewProp(N, BAD_CAST "is_restrict", BAD_CAST "1");
    isQualified = true;
  }
  if (isQualified) {
    registerType(T.getUnqualifiedType(), nullptr, traversingNode);
    xmlNewProp(N, BAD_CAST "name",
               BAD_CAST getTypeName(T.getUnqualifiedType()).c_str());
  }

  return N;
}

void TypeTableInfo::registerType(QualType T, xmlNodePtr *retNode, xmlNodePtr traversingNode) {
  bool isQualified = false;
  xmlNodePtr Node = nullptr;
  std::string rawname;

  if (T.isNull()) {
    return;
  };

  if (!T.isCanonical()) {
    if (OptTraceTypeTable) {
      xmlAddChild(traversingNode, xmlNewComment(BAD_CAST "registerType: not canonoical"));
    }
    registerType(T.getCanonicalType(), retNode, traversingNode);
    mapFromQualTypeToName[T] = mapFromQualTypeToName[T.getCanonicalType()];
    return;
  }

  if (!mapFromQualTypeToName[T].empty()) {
    if (retNode != nullptr) {
      *retNode = mapFromQualTypeToXmlNodePtr[T];
    }
    return;
  }

  if (T.isConstQualified()) {
    isQualified = true;
  }
  if (T.isVolatileQualified()) {
    isQualified = true;
  }
  if (T.isRestrictQualified()) {
    isQualified = true;
  }

  if (T->isBuiltinType() || T->getTypeClass() == Type::Builtin) {
    if (isQualified) {
      rawname = registerBasicType(T);
      // XXX: temporary imcompletearray
      Node = createNode(T, "basicType", traversingNode);
      basicTypeNodes.push_back(Node);
    } else {
      const Type *Tptr = T.getTypePtrOrNull();
      ASTContext &CXT = mangleContext->getASTContext();
      PrintingPolicy PP(CXT.getLangOpts());

      // XXX: temporary implementation
      rawname = static_cast<const BuiltinType*>(Tptr)->getName(PP).str();
      for (auto I = rawname.begin(); I != rawname.end(); ++I) {
        if (!std::isalnum(*I)) {
          *I = '_';
        }
      }
      mapFromQualTypeToName[T] = rawname;
    }
  } else switch (T->getTypeClass()) {
  case Type::Builtin:
    break;

  case Type::Complex:
    rawname = registerOtherType(T);
    // XXX: temporary implementation
    if (isQualified) {
      Node = createNode(T, "basicType", traversingNode);
    } else {
      Node = createNode(T, "complexType", traversingNode);
    }
    otherTypeNodes.push_back(Node);
    break;

  case Type::Pointer:
  case Type::BlockPointer:
  case Type::LValueReference:
  case Type::RValueReference:
  case Type::MemberPointer:
    {
      const PointerType *PT = dyn_cast<const PointerType>(T.getTypePtr());
      if (PT) {
        registerType(PT->getPointeeType(), nullptr, traversingNode);
      }
      rawname = registerPointerType(T);
      Node = createNode(T, "pointerType", traversingNode);
      if (PT) {
        xmlNewProp(Node, BAD_CAST "ref",
                   BAD_CAST getTypeName(PT->getPointeeType()).c_str());
      }
      pointerTypeNodes.push_back(Node);
    }
    break;

  case Type::ConstantArray:
    {
      const ConstantArrayType *CAT
        = dyn_cast<const ConstantArrayType>(T.getTypePtr());
      if (CAT) {
        registerType(CAT->getElementType(), nullptr, traversingNode);
      }
      rawname = registerArrayType(T);
      Node = createNode(T, "arrayType", traversingNode);
      if (CAT) {
        xmlNewProp(Node, BAD_CAST "element_type",
                   BAD_CAST getTypeName(CAT->getElementType()).c_str());
        xmlNewProp(Node, BAD_CAST "array_size",
                   BAD_CAST CAT->getSize().toString(10, false).c_str());
      }
      arrayTypeNodes.push_back(Node);
    }
    break;

  case Type::IncompleteArray:
  case Type::VariableArray:
  case Type::DependentSizedArray:
    {
      const ArrayType *AT = dyn_cast<const ArrayType>(T.getTypePtr());
      if (AT) {
        registerType(AT->getElementType(), nullptr, traversingNode);
      }
      rawname = registerArrayType(T);
      Node = createNode(T, "arrayType", traversingNode);
      if (AT){
        xmlNewProp(Node, BAD_CAST "element_type",
                   BAD_CAST getTypeName(AT->getElementType()).c_str());
      }
      arrayTypeNodes.push_back(Node);
    }
    break;

  case Type::DependentSizedExtVector:
  case Type::Vector:
  case Type::ExtVector:
    rawname = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "vectorType", traversingNode);
    arrayTypeNodes.push_back(Node);
    break;

  case Type::FunctionProto:
  case Type::FunctionNoProto:
    {
      const FunctionType *FT = dyn_cast<const FunctionType>(T.getTypePtr());
      if (FT) {
        registerType(FT->getReturnType(), nullptr, traversingNode);
      }
      rawname = registerFunctionType(T);
      Node = createNode(T, "functionType", traversingNode);
      if (FT) {
        xmlNewProp(Node, BAD_CAST "return_type",
                   BAD_CAST getTypeName(FT->getReturnType()).c_str());
      }
      functionTypeNodes.push_back(Node);
    }
    break;

  case Type::UnresolvedUsing:
  case Type::Paren:
  case Type::Typedef:
    rawname = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "typedefType", traversingNode);
    otherTypeNodes.push_back(Node);
    break;

  case Type::Adjusted:
  case Type::Decayed:
  case Type::TypeOfExpr:
  case Type::TypeOf:
  case Type::Decltype:
  case Type::UnaryTransform:
    rawname = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "otherType", traversingNode);
    otherTypeNodes.push_back(Node);
    break;

  case Type::Record:
    rawname = registerRecordType(T);
    if (T->isStructureType()) {
      Node = createNode(T, "structType", traversingNode);
      structTypeNodes.push_back(Node);
    } else if (T->isUnionType()) {
      Node = createNode(T, "unionType", traversingNode);
      unionTypeNodes.push_back(Node);
    } else if (T->isClassType()) {
      // XXX: temporary implementation
      Node = createNode(T, "classType", traversingNode);
      classTypeNodes.push_back(Node);
    } else {
      // XXX: temporary implementation
      Node = createNode(T, "unknownRecordType", traversingNode);
      otherTypeNodes.push_back(Node);
    }
    break;

  case Type::Enum:
    rawname = registerEnumType(T);
    Node = createNode(T, "enumType", traversingNode);
    enumTypeNodes.push_back(Node);
    break;

  case Type::Elaborated:
  case Type::Attributed:
  case Type::TemplateTypeParm:
  case Type::SubstTemplateTypeParm:
  case Type::SubstTemplateTypeParmPack:
  case Type::TemplateSpecialization:
  case Type::Auto:
  case Type::InjectedClassName:
  case Type::DependentName:
  case Type::DependentTemplateSpecialization:
  case Type::PackExpansion:
  case Type::ObjCObject:
  case Type::ObjCInterface:
  case Type::ObjCObjectPointer:
  case Type::Atomic:
    rawname = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "otherType", traversingNode);
    otherTypeNodes.push_back(Node);
    break;
  }

  if (retNode != nullptr) {
    *retNode = Node;
  }
  mapFromNameToQualType[rawname] = T;
  mapFromQualTypeToXmlNodePtr[T] = Node;

  if (traversingNode && OptTraceTypeTable) {
    std::string comment = "registerType: " + rawname + "(" + getTypeName(T) + ")";
    xmlAddChild(traversingNode, xmlNewComment(BAD_CAST comment.c_str()));
  }
}

void TypeTableInfo::registerLabelType(void)
{
  useLabelType = true;
}

std::string TypeTableInfo::getTypeName(QualType T)
{
  if (T.isNull()) {
    return "nullType";
  };

  std::string name = mapFromQualTypeToName[T];
  assert(!name.empty());

  if (!map_is_already_set) {
    typenamemap["unsigned_int"] = "unsigned";
  }
  if (!OptTypeNameMap.empty()) {
    if (!map_is_already_set) {
      std::cerr << "use " << OptTypeNameMap << " as a typenamemap file" << std::endl;
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
        //std::cerr << "typenamemap: " << lhs << "->" << rhs << std::endl;
      }
    }
    std::string rhs = typenamemap[name];
    return !rhs.empty() ? rhs : name;
  }

  map_is_already_set = true;
  return name;
}

std::string TypeTableInfo::getTypeNameForLabel(void)
{
  if (!typenamemap["Label"].empty()) {
    return typenamemap["Label"];
  } else {
    return "Label";
  }
}

void TypeTableInfo::emitAllTypeNode(xmlNodePtr ParentNode)
{
  for (xmlNodePtr N : basicTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : pointerTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  if (useLabelType) {
    xmlNodePtr N = xmlNewNode(nullptr, BAD_CAST "pointerType");
    xmlNewProp(N, BAD_CAST "type", BAD_CAST getTypeNameForLabel().c_str());
    xmlNewProp(N, BAD_CAST "ref", BAD_CAST "void");
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : arrayTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : structTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : unionTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : enumTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : classTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : functionTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : otherTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
}

std::vector<BaseClass> TypeTableInfo::getBaseClasses(clang::QualType type) {
  return inheritanceinfo->getInheritance(type);
}

void TypeTableInfo::addInheritance(clang::QualType derived, BaseClass base) {
  inheritanceinfo->addInheritance(derived, base);
}

bool TypeTableInfo::hasBaseClass(clang::QualType type) {
  return !( inheritanceinfo->getInheritance(type).empty() );
}

void TypeTableInfo::setNormalizability(clang::QualType T, bool b) {
  normalizability[T] = b;
}

bool TypeTableInfo::isNormalizable(clang::QualType T) {
  return normalizability[T];
}

const char *
TypeTableVisitor::getVisitorName() const {
  return OptTraceTypeTable ? "TypeTable" : nullptr;
}

bool
TypeTableVisitor::PreVisitStmt(Stmt *S) {
  if (OptTraceTypeTable) {
    newChild("TypeTableVisitor::PreVisitStmt");
  }
  if (!S) {
    return false;
  }
  Expr *E = dyn_cast<Expr>(S);

  if (E && S->getStmtClass() != Stmt::StringLiteralClass) {
    TraverseType(E->getType());
  }

  // additional typeinfo creation
  switch (S->getStmtClass()) {
  default:
    break;
  case Stmt::LabelStmtClass:
    typetableinfo->registerLabelType();
    break;
  case Stmt::UnaryExprOrTypeTraitExprClass: {
    UnaryExprOrTypeTraitExpr *UEOTTE = dyn_cast<UnaryExprOrTypeTraitExpr>(S);
    if (UEOTTE && UEOTTE->isArgumentType()) {
      TraverseType(UEOTTE->getArgumentType());
    }
    break;
  }
  case Stmt::ArraySubscriptExprClass:
  case Stmt::CallExprClass:
  case Stmt::DeclRefExprClass: {
    ASTContext &CXT = mangleContext->getASTContext();
    Expr *E = static_cast<Expr*>(S);
    TraverseType(CXT.getPointerType(E->getType()));
    break;
  }
  }
  return true; // do not create a new child
}

bool
TypeTableVisitor::PreVisitType(QualType T) {
  if (OptTraceTypeTable) {
    newChild("TypeTableVisitor::PreVisitType");
  }

  if (T.isNull()) {
    return false;
  };
  typetableinfo->registerType(T, nullptr, curNode);

  return true;
}

static bool isNested(const CXXRecordDecl &RD) {
  return RD.getParent()->getOuterLexicalRecordContext();
}

bool
TypeTableVisitor::PreVisitDecl(Decl *D) {
  if (OptTraceTypeTable) {
    newChild("PreVisitDecl");
  }

  if (!D) {
    return false;
  }
  if (D->isImplicit()) {
    return false;
  }

  switch (D->getKind()) {
  case Decl::AccessSpec: return true;
  case Decl::Block: return true;
  case Decl::Captured: return true;
  case Decl::ClassScopeFunctionSpecialization: return true;
  case Decl::Empty: return true;
  case Decl::FileScopeAsm: return true;
  case Decl::Friend: return true;
  case Decl::FriendTemplate: return true;
  case Decl::Import: return true;
  case Decl::LinkageSpec: return true;
  case Decl::Label:
    typetableinfo->registerLabelType();
    return true;
  case Decl::Namespace: return true;
  case Decl::NamespaceAlias: return true;
  case Decl::ObjCCompatibleAlias: return true;
  case Decl::ObjCCategory: return true;
  case Decl::ObjCCategoryImpl: return true;
  case Decl::ObjCImplementation: return true;
  case Decl::ObjCInterface: return true;
  case Decl::ObjCProtocol: return true;
  case Decl::ObjCMethod: return true;
  case Decl::ObjCProperty: return true;
  case Decl::ClassTemplate: return true;
  case Decl::FunctionTemplate: return true;
  case Decl::TypeAliasTemplate: return true;
  case Decl::VarTemplate: return true;
  case Decl::TemplateTemplateParm: return true;
  case Decl::Enum:
    {
      TagDecl *TD = dyn_cast<TagDecl>(D);
      if (!TD) {
        return false;
      }
      QualType T(TD->getTypeForDecl(), 0);
      if (TD->isCompleteDefinition()) {
        xmlNodePtr tmpNode;
        newComment("PreVisitDecl::Enum(withDef)");
        typetableinfo->registerType(T, &tmpNode, curNode);
        TraverseChildOfDecl(D);
        SymbolsVisitor SV(mangleContext, tmpNode, "symbols", typetableinfo);
        SV.TraverseChildOfDecl(D);
        return false;
      } else {
        // just allocate a name.
        newComment("PreVisitDecl::Enum");
        typetableinfo->registerType(T, nullptr, curNode);
        return true;
      }
    }
  case Decl::Record:
  case Decl::CXXRecord:
    {
      std::string comment("PreVisitDecl::");
      comment += D->getDeclKindName();
      TagDecl *TD = dyn_cast<TagDecl>(D);
      if (!TD) {
        return false;
      }
      QualType T(TD->getTypeForDecl(), 0);
      if (TD->isCompleteDefinition()) {
        xmlNodePtr tmpNode;
        newComment((comment + "(withDef)").c_str());
        typetableinfo->registerType(T, &tmpNode, curNode);
        curNode = tmpNode;
        addName(TD, TD->getNameAsString().c_str());
        CXXRecordDecl *RD(dyn_cast<CXXRecordDecl>(D));
        if (RD && RD->bases_begin() != RD->bases_end()) {
          for (auto base : RD->bases()) {
            BaseClass baseClass(base.getType(), base.getAccessSpecifier(), base.isVirtual());
            typetableinfo->addInheritance(T, baseClass);
          }
          xmlNodePtr basesNode = xmlNewNode(nullptr, BAD_CAST "inheritedFrom");
          for (BaseClass baseClass : typetableinfo->getBaseClasses(T)) {
            std::string name = typetableinfo->getTypeName(baseClass.type());
            xmlNodePtr typeNameNode = xmlNewNode(nullptr, BAD_CAST "typeName");
            xmlNewProp(typeNameNode, BAD_CAST "ref", BAD_CAST name.c_str());
            xmlNewProp(typeNameNode, BAD_CAST "access", BAD_CAST baseClass.access().c_str());
            if (baseClass.isVirtual()) {
              xmlNewProp(typeNameNode, BAD_CAST "is_virtual", BAD_CAST "1");
            }
            xmlAddChild(basesNode, typeNameNode);
          }
          xmlAddChild(tmpNode, basesNode);
        }
        if (RD->isLocalClass()) {
          typetableinfo->setNormalizability(T, false);
        } else if (! isNested(*RD)) {
          typetableinfo->setNormalizability(T, true);
        } else {
          /* CXXRecordDecl D is not local but in another class,
           * so D and the enblacing class is not normalizable.
           */
          RecordDecl* enclosure(D->getDeclContext()->getOuterLexicalRecordContext());
          QualType enclosureType(enclosure->getTypeForDecl(), 0);
          typetableinfo->setNormalizability(enclosureType, false);
          typetableinfo->setNormalizability(T, false);
        }
        TraverseChildOfDecl(D);
        SymbolsVisitor SV(mangleContext, tmpNode, "symbols", typetableinfo);
        SV.TraverseChildOfDecl(D);
        return false;
      } else {
        // just allocate a name.
        newComment(comment.c_str());
        typetableinfo->registerType(T, nullptr, curNode);
        return true;
      }
    }
  case Decl::ClassTemplateSpecialization: return true;
  case Decl::ClassTemplatePartialSpecialization: return true;
  case Decl::TemplateTypeParm: return true;
  case Decl::TypeAlias: return true;
  case Decl::Typedef:
    {
      TypedefDecl *TD = dyn_cast<TypedefDecl>(D);
      if (TD) {
        QualType T = TD->getUnderlyingType();
        typetableinfo->registerType(T, nullptr, curNode);
      }
      return true;
    }
  case Decl::UnresolvedUsingTypename: return true;
  case Decl::Using: return true;
  case Decl::UsingDirective: return true;
  case Decl::UsingShadow: return true;
  case Decl::Field:
    {
      FieldDecl *FD = dyn_cast<FieldDecl>(D);
      if (FD) {
        typetableinfo->registerType(FD->getType(), nullptr, curNode);
      }
      return true;
    }
  case Decl::ObjCAtDefsField: return true;
  case Decl::ObjCIvar: return true;
  case Decl::Function:
  case Decl::CXXMethod:
  case Decl::CXXConstructor:
  case Decl::CXXConversion:
  case Decl::CXXDestructor:
    {
      FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
      if (!FD) {
        return true;
      }
      typetableinfo->registerType(FD->getReturnType(), nullptr, curNode);
      QualType T = FD->getType();

      xmlNodePtr tmpNode;
      if (!FD->isFirstDecl()) {
        newComment(make_comment(D,
              FD->hasPrototype() ?
                "(with proto, not 1st)" :
                "(without proto, not 1st)"));
      } else {
        newComment(make_comment(D,
              FD->hasPrototype() ? "(with proto)" : "(without proto)"));
      }
      typetableinfo->registerType(T, &tmpNode, curNode);
      // quick hack
      if (xmlChildElementCount(tmpNode) == 0) {
        curNode = tmpNode;
        newChild("params");
      } else {
        newComment("PreVisitDecl::Function: already the same type is registered");
        return false;
      }
      return true;
#if 0
      TraverseChildOfDecl(D);
      SymbolsVisitor SV(mangleContext, tmpNode, "params", typetableinfo);
      SV.TraverseChildOfDecl(D);
      return false;
#endif
    }
  case Decl::MSProperty: return true;
  case Decl::NonTypeTemplateParm: return true;
  case Decl::Var:
    {
      VarDecl *VD = dyn_cast<VarDecl>(D);
      if (VD) {
        typetableinfo->registerType(VD->getType(), nullptr, curNode);
      }
      return true;
    }
  case Decl::ImplicitParam: return true;
  case Decl::ParmVar:
    {
      ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(D);
      if (PVD) {
        typetableinfo->registerType(PVD->getType(), nullptr, curNode);
        newChild("name", PVD->getNameAsString().c_str());
        newProp("type", typetableinfo->getTypeName(PVD->getType()).c_str());
      }
      return false;
      //return true;
    }
  case Decl::VarTemplateSpecialization: return true;
  case Decl::VarTemplatePartialSpecialization: return true;
  case Decl::EnumConstant: return true;
  case Decl::IndirectField: return true;
  case Decl::UnresolvedUsingValue: return true;
  case Decl::OMPThreadPrivate: return true;
  case Decl::ObjCPropertyImpl: return true;
  case Decl::StaticAssert: return true;
  case Decl::TranslationUnit:
    if (!OptDisableTypeTable) {
      TraverseChildOfDecl(D);
      typetableinfo->emitAllTypeNode(curNode);
    }
    return false; // already traversed: stop traversing
  }
  return true; // do not create a new child
}

bool
TypeTableVisitor::PreVisitAttr(Attr *A) {
  if (OptTraceTypeTable) {
    newChild("TypeTableVisitor::PreVisitAttr");
  }

  (void)A;
  return true;
}

bool
TypeTableVisitor::PreVisitTypeLoc(TypeLoc TL) {
  if (OptTraceTypeTable) {
    newChild("TypeTableVisitor::PreVisitTypeLoc");
  }

  (void)TL;
  return true;
}

bool
TypeTableVisitor::PreVisitNestedNameSpecifierLoc(NestedNameSpecifierLoc N) {
  if (OptTraceTypeTable) {
    newChild("TypeTableVisitor::PreVisitNextedNameSpecifierLoc");
  }

  (void)N;
  return true;
}

bool
TypeTableVisitor::PreVisitDeclarationNameInfo(DeclarationNameInfo NI) {
  if (OptTraceTypeTable) {
    newChild("TypeTableVisitor::PreVisitDeclarationNameInfo");
  }

  (void)NI;
  return true;
}

bool
TypeTableVisitor::PreVisitConstructorInitializer(CXXCtorInitializer *) {
  return true;
}
///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
