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

#define TArray()    OS << "A" << seqForArrayType++; break

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

std::string TypeTableInfo::registerArrayType(QualType T, long *arraysize){
  std::string name = mapFromQualTypeToName[T];
  assert(name.empty());

  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::ConstantArray:
    OS << "A" << seqForArrayType++;
    if (arraysize != nullptr) {
      APInt Value = static_cast<const ConstantArrayType *>(T.getTypePtr())->getSize();
      *arraysize = *Value.getRawData();
    }
    break;
  case Type::IncompleteArray:
    OS << "A" << seqForArrayType++;
    if (arraysize != nullptr) {
      *arraysize = -1; /* unknown array size */
    }
    break;
  case Type::VariableArray:
    OS << "A" << seqForArrayType++;
    if (arraysize != nullptr) {
      *arraysize = -1; /* unknown array size */
    }
    break;
  case Type::DependentSizedArray:
    OS << "A" << seqForArrayType++;
    if (arraysize != nullptr) {
      *arraysize = -1; /* unknown array size */
    }
    break;
  default:
    abort();
  }
  return mapFromQualTypeToName[T] = OS.str();
}

std::string TypeTableInfo::registerRecordType(QualType T, std::string *rawname){
  assert(T->getTypeClass() == Type::Record);
  std::string name = mapFromQualTypeToName[T];

  if (name.empty()) {
    return name; // XXX: quick dirty hack
  }
  if (rawname != nullptr) {
    std::string rawnamebuf;
    raw_string_ostream OS(rawnamebuf);

    if (T->isStructureType()) {
      OS << "S" << seqForStructType++;
    } else if (T->isUnionType()) {
      OS << "U" << seqForUnionType++;
    } else if (T->isClassType()) {
      OS << "C" << seqForClassType++;
    } else {
      abort();
    }
    *rawname = OS.str();
  }
  return registerBasicType(T);
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

std::string TypeTableInfo::getTypeName(QualType T, bool *created){
  if (created) {
    *created = false;
  }
  if (T.isNull()) {
    return "nullType";
  };
  std::string name = mapFromQualTypeToName[T];
  if (!name.empty()) {
    return name;
  }

  if (created) {
    *created = true;
  }
  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::Builtin: {
    const Type *Tptr = T.getTypePtrOrNull();
    ASTContext &CXT = mangleContext->getASTContext();
    PrintingPolicy PP(CXT.getLangOpts());
    return mapFromQualTypeToName[T]
      = static_cast<const BuiltinType*>(Tptr)->getName(PP).str();
  }
  case Type::Complex: return registerOtherType(T);
  case Type::Pointer: return registerPointerType(T);
  case Type::BlockPointer: return registerPointerType(T);
  case Type::LValueReference: return registerPointerType(T);
  case Type::RValueReference: return registerPointerType(T);
  case Type::MemberPointer: return registerPointerType(T);
  case Type::ConstantArray: return registerArrayType(T);
  case Type::IncompleteArray: return registerArrayType(T);
  case Type::VariableArray: return registerArrayType(T);
  case Type::DependentSizedArray: return registerArrayType(T);
  case Type::DependentSizedExtVector: return registerOtherType(T);
  case Type::Vector: return registerOtherType(T);
  case Type::ExtVector: return registerOtherType(T);
  case Type::FunctionProto: return registerFunctionType(T);
  case Type::FunctionNoProto: return registerFunctionType(T);
  case Type::UnresolvedUsing: return registerOtherType(T);
  case Type::Paren: return registerOtherType(T);
  case Type::Typedef: return registerOtherType(T);
  case Type::Adjusted: return registerOtherType(T);
  case Type::Decayed: return registerOtherType(T);
  case Type::TypeOfExpr: return registerOtherType(T);
  case Type::TypeOf: return registerOtherType(T);
  case Type::Decltype: return registerOtherType(T);
  case Type::UnaryTransform: return registerOtherType(T);
  case Type::Record: return registerRecordType(T);
  case Type::Enum: return registerEnumType(T);
  case Type::Elaborated: return registerOtherType(T);
  case Type::Attributed: return registerOtherType(T);
  case Type::TemplateTypeParm: return registerOtherType(T);
  case Type::SubstTemplateTypeParm: return registerOtherType(T);
  case Type::SubstTemplateTypeParmPack: return registerOtherType(T);
  case Type::TemplateSpecialization: return registerOtherType(T);
  case Type::Auto: return registerOtherType(T);
  case Type::InjectedClassName: return registerOtherType(T);
  case Type::DependentName: return registerOtherType(T);
  case Type::DependentTemplateSpecialization: return registerOtherType(T);
  case Type::PackExpansion: return registerOtherType(T);
  case Type::ObjCObject: return registerOtherType(T);
  case Type::ObjCInterface: return registerOtherType(T);
  case Type::ObjCObjectPointer: return registerOtherType(T);
  case Type::Atomic: return registerOtherType(T);
  }
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

  bool created;
  const char *Name = typetableinfo->getTypeName(T, &created).c_str();
  if (!created) {
    return true; // emit nothing
  }
  switch (Name[0]) {
  case 'P': {
    xmlNodePtr N = addChild("pointerType");
    newProp("type", Name, N);
    const PointerType *PT = dyn_cast<const PointerType>(T.getTypePtr());
    if (PT) {
      newProp("ref",
              typetableinfo->getTypeName(PT->getPointeeType()).c_str(), N);
    }
    break;
  }
  case 'F':
    newProp("type", Name, addChild("functionType"));
    break;
  case 'A':
    newProp("type", Name, addChild("arrayType"));
    break;
  case 'S':
    break;
  case 'U':
    break;
  case 'E':
    break;
  case 'C':
    break;
  case 'O':
    newProp("type", Name, addChild("otherType"));
    break;
  default:
    newProp("type", Name, addChild("basicType"));
    break;
  }
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
    if (TD && TD->isCompleteDefinition()) {
      QualType T(TD->getTypeForDecl(), 0);
      newChild("enumType");
      bool created;
      const char *Name = typetableinfo->getTypeName(T, &created).c_str();
      newProp("type", Name);
      SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
      SV.TraverseChildOfDecl(D);
    }
    return true;
  }
  case Decl::Record: {
    TagDecl *TD = dyn_cast<TagDecl>(D);
    if (!TD) {
      return false;
    }
    std::string RawName, Name;
    QualType T(TD->getTypeForDecl(), 0);

    Name = typetableinfo->registerRecordType(T, &RawName);
    xmlNodePtr origCurNode = curNode;
    newChild("basicType");
    newProp("type", Name.c_str());
    curNode = origCurNode;

    if (TD->isCompleteDefinition()) {
      if (T->isStructureType()) {
        newChild("structType");
      } else if (T->isUnionType()) {
        newChild("unionType");
      } else if (T->isClassType()) {
        newChild("classType");
      } else {
        newChild("UnknownRecordType");
      }
      newProp("type", RawName.c_str());
      SymbolsVisitor SV(mangleContext, curNode, "symbols", typetableinfo);
      SV.TraverseChildOfDecl(D);
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
