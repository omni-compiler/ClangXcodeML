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
  seqForBasicType = 0;
  seqForPointerType = 0;
  seqForFunctionType = 0;
  seqForArrayType = 0;
  seqForStructType = 0;
  seqForUnionType = 0;
  seqForEnumType = 0;
  seqForClassType = 0;
  seqForOtherType = 0;
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
  if (!name.empty()) {
    return name;
  }

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

std::string TypeTableInfo::createTypeNode(QualType T, xmlNodePtr *N, bool force){
  if (T.isNull()) {
    return "nullType";
  };
  std::string name = mapFromQualTypeToName[T];
  bool toCreate = force;

  assert(!(N == nullptr && name.empty()));

  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::Builtin:
    {
      const Type *Tptr = T.getTypePtrOrNull();
      ASTContext &CXT = mangleContext->getASTContext();
      PrintingPolicy PP(CXT.getLangOpts());
      if (name.empty()) {
        name
          = mapFromQualTypeToName[T]
          = static_cast<const BuiltinType*>(Tptr)->getName(PP).str();
      }
    }
    break;

  case Type::Complex:
    {
      if (name.empty()) {
        name = registerOtherType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        // XXX: temporary implementation
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "complexType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
    break;

  case Type::Pointer:
  case Type::BlockPointer:
  case Type::LValueReference:
  case Type::RValueReference:
  case Type::MemberPointer:
    {
      const PointerType *PT = dyn_cast<const PointerType>(T.getTypePtr());
      if (PT && N != nullptr && *N != nullptr) {
        xmlNodePtr Tmp = *N;
        createTypeNode(PT->getPointeeType(), &Tmp);
      }
      if (name.empty()) {
        name = registerPointerType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "pointerType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
        if (PT) {
          xmlNewProp(*N, BAD_CAST "ref",
                     BAD_CAST getTypeName(PT->getPointeeType()).c_str());
        }
      }
    }
    break;

  case Type::ConstantArray:
    {
      const ConstantArrayType *CAT
        = dyn_cast<const ConstantArrayType>(T.getTypePtr());
      if (CAT && N != nullptr && *N != nullptr) {
        xmlNodePtr Tmp = *N;
        createTypeNode(CAT->getElementType(), &Tmp);
      }

      if (name.empty()) {
        name = registerArrayType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "arrayType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
        if (CAT) {
          xmlNewProp(*N, BAD_CAST "element_type",
                     BAD_CAST getTypeName(CAT->getElementType()).c_str());
          xmlNewProp(*N, BAD_CAST "array_size",
                     BAD_CAST CAT->getSize().toString(10, false).c_str());
        }
      }
    }
    break;

  case Type::IncompleteArray:
  case Type::VariableArray:
  case Type::DependentSizedArray:
    {
      const ArrayType *AT = dyn_cast<const ArrayType>(T.getTypePtr());
      if (AT && N != nullptr && *N != nullptr) {
        xmlNodePtr Tmp = *N;
        createTypeNode(AT->getElementType(), &Tmp);
      }
      if (name.empty()) {
        name = registerArrayType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "arrayType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
        if (AT) {
          xmlNewProp(*N, BAD_CAST "element_type",
                     BAD_CAST getTypeName(AT->getElementType()).c_str());
        }
      }
    }
    break;

  case Type::DependentSizedExtVector:
  case Type::Vector:
  case Type::ExtVector:
    {
      if (name.empty()) {
        name = registerOtherType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        // XXX: temporary implementation
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "vectorType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
    break;

  case Type::FunctionProto:
  case Type::FunctionNoProto:
    {
      if (name.empty()) {
        name = registerFunctionType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "functionType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
    break;

  case Type::UnresolvedUsing:
  case Type::Paren:
  case Type::Typedef:
    {
      if (name.empty()) {
        name = registerOtherType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        // XXX: temporary implementation
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "typedefType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
    break;

  case Type::Adjusted:
  case Type::Decayed:
  case Type::TypeOfExpr:
  case Type::TypeOf:
  case Type::Decltype:
  case Type::UnaryTransform:
    {
      if (name.empty()) {
        name = registerOtherType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        // XXX: temporary implementation
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "otherType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
    break;

  case Type::Record:
    {
      if (name.empty()) {
        name = registerRecordType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        if (T->isStructureType()) {
          *N = xmlNewTextChild(*N, nullptr, BAD_CAST "structType", nullptr);
        } else if (T->isUnionType()) {
          *N = xmlNewTextChild(*N, nullptr, BAD_CAST "unionType", nullptr);
        } else if (T->isClassType()) {
          // XXX: temporary implementation
          *N = xmlNewTextChild(*N, nullptr, BAD_CAST "classType", nullptr);
        } else {
          // XXX: temporary implementation
          *N = xmlNewTextChild(*N, nullptr, BAD_CAST "unknownRecordType", nullptr);
        }
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
    break;

  case Type::Enum:
    {
      if (name.empty()) {
        name = registerEnumType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "enumType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
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
    {
      if (name.empty()) {
        name = registerOtherType(T);
        toCreate = true;
      }
      if (toCreate && N != nullptr && *N != nullptr) {
        // XXX: temporary implementation
        *N = xmlNewTextChild(*N, nullptr, BAD_CAST "otherType", nullptr);
        xmlNewProp(*N, BAD_CAST "type", BAD_CAST name.c_str());
      }
    }
    break;
  }

  return name;
}

std::string TypeTableInfo::getTypeName(QualType T)
{
  return createTypeNode(T, nullptr);
}

const char *
TypeTableVisitor::getVisitorName() const {
  return OptTraceTypeTable ? "TypeTable" : nullptr;
}

bool
TypeTableVisitor::PreVisitStmt(Stmt *S) {
  if (!S) {
    return false;
  }
  Expr *E = dyn_cast<Expr>(S);

  if (E) {
    TraverseType(E->getType());
  }
  return true; // do not create a new child
}

bool
TypeTableVisitor::PreVisitType(QualType T) {
  newComment("PreVisitType");

  if (T.isNull()) {
    return false;
  };
  xmlNodePtr Tmp = curNode;
  typetableinfo->createTypeNode(T, &Tmp);
  return true;
}

#define DECLTYPE()                              \
  do {                                          \
    TypeDecl *TD = dyn_cast<TypeDecl>(D);       \
    if (TD) {                                   \
      QualType T(TD->getTypeForDecl(), 0);      \
      PreVisitType(T);                          \
    }                                           \
  } while (0)

bool
TypeTableVisitor::PreVisitDecl(Decl *D) {
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
  case Decl::Enum: {
    TagDecl *TD = dyn_cast<TagDecl>(D);
    if (!TD) {
      return false;
    }
    QualType T(TD->getTypeForDecl(), 0);
    if (TD->isCompleteDefinition()) {
      typetableinfo->createTypeNode(T, &curNode, true);
      SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
      SV.TraverseChildOfDecl(D);
    } else {
      // reserve a new name (but no nodes are emitted)
      xmlNodePtr dummyNode = nullptr;
      typetableinfo->createTypeNode(T, &dummyNode);
    }
    return true;
  }
  case Decl::Record: {
    TagDecl *TD = dyn_cast<TagDecl>(D);
    if (!TD) {
      return false;
    }
    QualType T(TD->getTypeForDecl(), 0);
    if (TD->isCompleteDefinition()) {
      typetableinfo->createTypeNode(T, &curNode, true);
      SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
      SV.TraverseChildOfDecl(D);
    } else {
      // reserve a new name (but no nodes are emitted)
      xmlNodePtr dummyNode = nullptr;
      typetableinfo->createTypeNode(T, &dummyNode);
    }
    return true;
  }
  case Decl::CXXRecord: return true;
  case Decl::ClassTemplateSpecialization: return true;
  case Decl::ClassTemplatePartialSpecialization: return true;
  case Decl::TemplateTypeParm: return true;
  case Decl::TypeAlias: return true;
  case Decl::Typedef: DECLTYPE(); return true;
  case Decl::UnresolvedUsingTypename: return true;
  case Decl::Using: return true;
  case Decl::UsingDirective: return true;
  case Decl::UsingShadow: return true;
  case Decl::Field: return true;
  case Decl::ObjCAtDefsField: return true;
  case Decl::ObjCIvar: return true;
  case Decl::Function: DECLTYPE(); return true;
  case Decl::CXXMethod: return true;
  case Decl::CXXConstructor: return true;
  case Decl::CXXConversion: return true;
  case Decl::CXXDestructor: return true;
  case Decl::MSProperty: return true;
  case Decl::NonTypeTemplateParm: return true;
  case Decl::Var: DECLTYPE(); return true;
  case Decl::ImplicitParam: return true;
  case Decl::ParmVar: return true;
  case Decl::VarTemplateSpecialization: return true;
  case Decl::VarTemplatePartialSpecialization: return true;
  case Decl::EnumConstant: return true;
  case Decl::IndirectField: return true;
  case Decl::UnresolvedUsingValue: return true;
  case Decl::OMPThreadPrivate: return true;
  case Decl::ObjCPropertyImpl: return true;
  case Decl::StaticAssert: return true;
  case Decl::TranslationUnit:
    if (OptDisableTypeTable) {
      return false; // stop traverse
    } else {
      return true; // no need to create a child
    }
  }
  return true; // do not create a new child
}

bool
TypeTableVisitor::PreVisitAttr(Attr *A) {
  (void)A;
  return true;
}

bool
TypeTableVisitor::PreVisitTypeLoc(TypeLoc TL) {
  (void)TL;
  return true;
}

bool
TypeTableVisitor::PreVisitNestedNameSpecifierLoc(NestedNameSpecifierLoc N) {
  (void)N;
  return true;
}

bool
TypeTableVisitor::PreVisitDeclarationNameInfo(DeclarationNameInfo NI) {
  (void)NI;
  return true;
}
///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
