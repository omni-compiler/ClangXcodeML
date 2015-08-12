#include "XcodeMlVisitorBase.h"
#include "SymbolsVisitor.h"
#include "TypeTableVisitor.h"

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

bool
TypeTableVisitor::FullTrace(void) const {
  return OptFullTraceTypeTable;
}

TypeTableInfo::TypeTableInfo(MangleContext *MC) : mangleContext(MC)
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
}

std::string TypeTableInfo::registerBasicType(QualType T){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "B" << seqForBasicType++;
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
    OS << "P" << seqForPointerType++;
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
  OS << "F" << seqForFunctionType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerArrayType(QualType T){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::ConstantArray:
    OS << "A" << seqForArrayType++;
    break;
  case Type::IncompleteArray:
    OS << "A" << seqForArrayType++;
    break;
  case Type::VariableArray:
    OS << "A" << seqForArrayType++;
    break;
  case Type::DependentSizedArray:
    OS << "A" << seqForArrayType++;
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
    OS << "S" << seqForStructType++;
  } else if (T->isUnionType()) {
    OS << "U" << seqForUnionType++;
  } else if (T->isClassType()) {
    OS << "C" << seqForClassType++;
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
  OS << "E" << seqForEnumType++;
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerOtherType(QualType T){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  OS << "O" << seqForOtherType++;
  return mapFromQualTypeToName[T] = OS.str();
}

xmlNodePtr TypeTableInfo::createNode(QualType T, const char *fieldname, std::string name, xmlNodePtr traversingNode) {
  xmlNodePtr N = xmlNewNode(nullptr, BAD_CAST fieldname);
  bool isQualified = false;
  xmlNewProp(N, BAD_CAST "type", BAD_CAST name.c_str());

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
    std::string unqualname = registerType(T.getUnqualifiedType(), nullptr, traversingNode);
    xmlNewProp(N, BAD_CAST "name", BAD_CAST unqualname.c_str());
  }

  return N;
}

std::string TypeTableInfo::registerType(QualType T, xmlNodePtr *retNode, xmlNodePtr traversingNode) {
  bool isQualified = false;
  xmlNodePtr Node = nullptr;
  std::string name = mapFromQualTypeToName[T];

  if (T.isNull()) {
    return "nullType";
  };

  if (!T.isCanonical()) {
    if (OptTraceTypeTable) {
      xmlAddChild(traversingNode, xmlNewComment(BAD_CAST "registerType: not canonoical"));
    }
    std::string name = registerType(T.getCanonicalType(), retNode, traversingNode);
    mapFromQualTypeToName[T] = name;
    return name;
  }

  if (!name.empty()) {
    if (retNode != nullptr) {
      *retNode = mapFromQualTypeToXmlNodePtr[T];
    }
    return name;
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

  switch (T->getTypeClass()) {
  case Type::Builtin:
    if (isQualified) {
      name = registerBasicType(T);
      Node = createNode(T, "basicType", name, traversingNode);
      basicTypeNodes.push_back(Node);
    } else {
      const Type *Tptr = T.getTypePtrOrNull();
      ASTContext &CXT = mangleContext->getASTContext();
      PrintingPolicy PP(CXT.getLangOpts());

      return (mapFromQualTypeToName[T]
              = static_cast<const BuiltinType*>(Tptr)->getName(PP).str());
    }
    break;

  case Type::Complex:
    name = registerOtherType(T);
    // XXX: temporary implementation
    if (isQualified) {
      Node = createNode(T, "basicType", name, traversingNode);
    } else {
      Node = createNode(T, "complexType", name, traversingNode);
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
      std::string pointeeName = "";
      if (PT) {
        pointeeName = registerType(PT->getPointeeType(), nullptr, traversingNode);
      }
      name = registerPointerType(T);
      Node = createNode(T, "pointerType", name, traversingNode);
      if (PT) {
        xmlNewProp(Node, BAD_CAST "ref", BAD_CAST pointeeName.c_str());
      }
      pointerTypeNodes.push_back(Node);
    }
    break;

  case Type::ConstantArray:
    {
      const ConstantArrayType *CAT
        = dyn_cast<const ConstantArrayType>(T.getTypePtr());
      std::string elementName = "";
      if (CAT) {
        elementName = registerType(CAT->getElementType(), nullptr, traversingNode);
      }
      name = registerArrayType(T);
      Node = createNode(T, "arrayType", name, traversingNode);
      if (CAT) {
        xmlNewProp(Node, BAD_CAST "element_type",
                   BAD_CAST registerType(CAT->getElementType(), nullptr, traversingNode).c_str());
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
      std::string elementName = "";
      if (AT) {
        elementName = registerType(AT->getElementType(), nullptr, traversingNode);
      }
      name = registerArrayType(T);
      Node = createNode(T, "arrayType", name, traversingNode);
      if (AT){
        xmlNewProp(Node, BAD_CAST "element_type",
                   BAD_CAST registerType(AT->getElementType(), nullptr, traversingNode).c_str());
      }
      arrayTypeNodes.push_back(Node);
    }
    break;

  case Type::DependentSizedExtVector:
  case Type::Vector:
  case Type::ExtVector:
    name = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "vectorType", name, traversingNode);
    arrayTypeNodes.push_back(Node);
    break;

  case Type::FunctionProto:
  case Type::FunctionNoProto:
    {
      const FunctionType *FT = dyn_cast<const FunctionType>(T.getTypePtr());
      std::string returnType = "";
      if (FT) {
        returnType = registerType(FT->getReturnType(), nullptr, traversingNode);
      }
      name = registerFunctionType(T);
      Node = createNode(T, "functionType", name, traversingNode);
      xmlNewProp(Node, BAD_CAST "return_type", BAD_CAST returnType.c_str());
      functionTypeNodes.push_back(Node);
    }
    break;

  case Type::UnresolvedUsing:
  case Type::Paren:
  case Type::Typedef:
    name = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "typedefType", name, traversingNode);
    otherTypeNodes.push_back(Node);
    break;

  case Type::Adjusted:
  case Type::Decayed:
  case Type::TypeOfExpr:
  case Type::TypeOf:
  case Type::Decltype:
  case Type::UnaryTransform:
    name = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "otherType", name, traversingNode);
    otherTypeNodes.push_back(Node);
    break;

  case Type::Record:
    name = registerRecordType(T);
    if (T->isStructureType()) {
      Node = createNode(T, "structType", name, traversingNode);
      structTypeNodes.push_back(Node);
    } else if (T->isUnionType()) {
      Node = createNode(T, "unionType", name, traversingNode);
      unionTypeNodes.push_back(Node);
    } else if (T->isClassType()) {
      // XXX: temporary implementation
      Node = createNode(T, "classType", name, traversingNode);
      classTypeNodes.push_back(Node);
    } else {
      // XXX: temporary implementation
      Node = createNode(T, "unknownRecordType", name, traversingNode);
      otherTypeNodes.push_back(Node);
    }
    break;

  case Type::Enum:
    name = registerEnumType(T);
    Node = createNode(T, "enumType", name, traversingNode);
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
    name = registerOtherType(T);
    // XXX: temporary implementation
    Node = createNode(T, "otherType", name, traversingNode);
    otherTypeNodes.push_back(Node);
    break;
  }

  if (retNode != nullptr) {
    *retNode = Node;
  }
  mapFromNameToQualType[name] = T;
  mapFromQualTypeToXmlNodePtr[T] = Node;

  if (traversingNode && OptTraceTypeTable) {
    std::string comment = "registerType: " + name;
    xmlAddChild(traversingNode, xmlNewComment(BAD_CAST comment.c_str()));
  }

  return name;
}

std::string TypeTableInfo::getTypeName(QualType T)
{
  if (T.isNull()) {
    return "nullType";
  };
  std::string name = mapFromQualTypeToName[T];

  assert(!name.empty());
  return name;
}

void TypeTableInfo::emitAllTypeNode(xmlNodePtr ParentNode)
{
  for (xmlNodePtr N : basicTypeNodes) {
    xmlAddChild(ParentNode, N);
  }
  for (xmlNodePtr N : pointerTypeNodes) {
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

bool
TypeTableVisitor::PreVisitDecl(Decl *D) {
  if (OptTraceTypeTable) {
    newChild("PreVisitDecl");
  }

  if (!D) {
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
  case Decl::Label: return true;
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
    {
      TagDecl *TD = dyn_cast<TagDecl>(D);
      if (!TD) {
        return false;
      }
      QualType T(TD->getTypeForDecl(), 0);
      if (TD->isCompleteDefinition()) {
        xmlNodePtr tmpNode;
        newComment("PreVisitDecl::Record(withDef)");
        typetableinfo->registerType(T, &tmpNode, curNode);
        TraverseChildOfDecl(D);
        SymbolsVisitor SV(mangleContext, tmpNode, "symbols", typetableinfo);
        SV.TraverseChildOfDecl(D);
        return false;

      } else {
        // just allocate a name.
        newComment("PreVisitDecl::Record");
        typetableinfo->registerType(T, nullptr, curNode);
        return true;
      }
    }
  case Decl::CXXRecord: return true;
  case Decl::ClassTemplateSpecialization: return true;
  case Decl::ClassTemplatePartialSpecialization: return true;
  case Decl::TemplateTypeParm: return true;
  case Decl::TypeAlias: return true;
  case Decl::Typedef:
    {
      TypedefDecl *TD = dyn_cast<TypedefDecl>(D);
      if (TD) {
        QualType T = TD->getUnderlyingType();
        PreVisitType(T);
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
        PreVisitType(FD->getType());
      }
      return true;
    }
  case Decl::ObjCAtDefsField: return true;
  case Decl::ObjCIvar: return true;
  case Decl::Function:
    {
      TypeDecl *TD = dyn_cast<TypeDecl>(D);
      FunctionDecl *FD = dyn_cast<FunctionDecl>(D);
      if (FD) {
        PreVisitType(FD->getReturnType());
      }
      if (!TD) {
        return true;
      }
      QualType T(TD->getTypeForDecl(), 0);
      if (FD && FD->hasPrototype()) {
        xmlNodePtr tmpNode;
        newComment("PreVisitDecl::Function(withDef)");
        typetableinfo->registerType(T, &tmpNode, curNode);
        TraverseChildOfDecl(D);
        SymbolsVisitor SV(mangleContext, tmpNode, "params", typetableinfo);
        SV.TraverseChildOfDecl(D);
        return false;
      } else {
        // just allocate a name.
        newComment("PreVisitDecl::Function");
        typetableinfo->registerType(T, nullptr, curNode);
        return true;
      }
    }
  case Decl::CXXMethod: return true;
  case Decl::CXXConstructor: return true;
  case Decl::CXXConversion: return true;
  case Decl::CXXDestructor: return true;
  case Decl::MSProperty: return true;
  case Decl::NonTypeTemplateParm: return true;
  case Decl::Var:
    {
      VarDecl *VD = dyn_cast<VarDecl>(D);
      if (VD) {
        PreVisitType(VD->getType());
      }
      return true;
    }
  case Decl::ImplicitParam: return true;
  case Decl::ParmVar:
    {
      ParmVarDecl *PVD = dyn_cast<ParmVarDecl>(D);
      if (PVD) {
        PreVisitType(PVD->getType());
      }
      return true;
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
///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
