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
  seqForPointerType = 0;
  seqForFunctionType = 0;
  seqForArrayType = 0;
  seqForStructType = 0;
  seqForUnionType = 0;
  seqForEnumType = 0;
  seqForClassType = 0;
  seqForOtherType = 0;
}

#define TPointer()  OS << "P" << seqForPointerType++; break
#define TFunction() OS << "F" << seqForFunctionType++; break
#define TArray()    OS << "A" << seqForArrayType++; break
#define TStruct()   OS << "S" << seqForStructType++; break
#define TUnion()    OS << "U" << seqForUnionType++; break
#define TEnum()     OS << "E" << seqForEnumType++; break
#define TClass()    OS << "C" << seqForClassType++; break
#define TOther()    OS << "O" << seqForOtherType++; break

#define TReference()     TPointer()
#define TMemberPointer() TOther()

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
  case Type::Complex: TOther();
  case Type::Pointer: TPointer();
  case Type::BlockPointer: TPointer();
  case Type::LValueReference: TReference();
  case Type::RValueReference: TReference();
  case Type::MemberPointer: TMemberPointer();
  case Type::ConstantArray: TArray();
  case Type::IncompleteArray: TArray();
  case Type::VariableArray: TArray();
  case Type::DependentSizedArray: TArray();
  case Type::DependentSizedExtVector: TOther();
  case Type::Vector: TOther();
  case Type::ExtVector: TOther();
  case Type::FunctionProto: TFunction();
  case Type::FunctionNoProto: TFunction();
  case Type::UnresolvedUsing: TOther();
  case Type::Paren: TOther();
  case Type::Typedef: TOther();
  case Type::Adjusted: TOther();
  case Type::Decayed: TOther();
  case Type::TypeOfExpr: TOther();
  case Type::TypeOf: TOther();
  case Type::Decltype: TOther();
  case Type::UnaryTransform: TOther();
  case Type::Record: {
    if (T->isStructureType()) {
      TStruct();
    } else if (T->isUnionType()) {
      TUnion();
    } else if (T->isClassType()) {
      TClass();
    } else {
      TOther();
    }
  }
  case Type::Enum: TEnum();
  case Type::Elaborated: TOther();
  case Type::Attributed: TOther();
  case Type::TemplateTypeParm: TOther();
  case Type::SubstTemplateTypeParm: TOther();
  case Type::SubstTemplateTypeParmPack: TOther();
  case Type::TemplateSpecialization: TOther();
  case Type::Auto: TOther();
  case Type::InjectedClassName: TOther();
  case Type::DependentName: TOther();
  case Type::DependentTemplateSpecialization: TOther();
  case Type::PackExpansion: TOther();
  case Type::ObjCObject: TOther();
  case Type::ObjCInterface: TOther();
  case Type::ObjCObjectPointer: TOther();
  case Type::Atomic: TOther();
  }
  return mapFromQualTypeToName[T] = OS.str();
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
    if (TD && TD->isCompleteDefinition()) {
      QualType T(TD->getTypeForDecl(), 0);
      if (T->isStructureType()) {
        newChild("structType");
      } else if (T->isUnionType()) {
        newChild("unionType");
      } else if (T->isClassType()) {
        newChild("classType");
      } else {
        newChild("UnknownRecordType");
      }
      bool created;
      const char *Name = typetableinfo->getTypeName(T, &created).c_str();
      newProp("type", Name);
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
