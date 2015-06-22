#include "XcodeMlVisitorBase.h"
#include "TypeTableVisitor.h"

using namespace clang;
using namespace llvm;

static cl::opt<bool>
OptTraceTypeTable("trace-typeTable",
                  cl::desc("emit traces on <typeTable>"),
                  cl::cat(C2XcodeMLCategory));
static cl::opt<bool>
OptDisableTypeTable("disable-typeTable",
                    cl::desc("disable <typeTable>"),
                    cl::cat(C2XcodeMLCategory));

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

std::string TypeTableInfo::getTypeName(QualType T)
{
  if (T.isNull()) {
    return "nullType";
  };
  std::string name = mapFromQualTypeToName[T];
  if (!name.empty()) {
    return name;
  }
  raw_string_ostream OS(name);
  switch (T->getTypeClass()) {
  case Type::Builtin: {
    const Type *Tptr = T.getTypePtrOrNull();
    ASTContext &CXT = mangleContext->getASTContext();
    PrintingPolicy PP(CXT.getLangOpts());
    return static_cast<const BuiltinType*>(Tptr)->getName(PP).str();
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
    TStruct(); // or TUnion() or TClass() ...
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
  (void)S;
  return true; // do not create a new child
}

#define DECLTYPE() return true //empty
bool
TypeTableVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return false;
  }
  TypeDecl *TD = dyn_cast<TypeDecl>(D);
  if (TD) {
    QualType T(TD->getTypeForDecl(), 0);
    const char *Name = typetableinfo->getTypeName(T).c_str();
    switch (Name[0]) {
    case 'P': newChild("pointerType"); newProp("type", Name); break;
    case 'F': newChild("functionType"); newProp("type", Name); break;
    case 'A': newChild("arrayType"); newProp("type", Name); break;
    case 'S': newChild("structType"); newProp("type", Name); break;
    case 'U': newChild("unionType"); newProp("type", Name); break;
    case 'E': newChild("enumType"); newProp("type", Name); break;
    case 'C': newChild("classType"); newProp("type", Name); break;
    case 'O': newChild("otherType"); newProp("type", Name); break;
    default: newChild("basicType"); newProp("type", Name); break;
    }
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
  case Decl::Enum: DECLTYPE();
  case Decl::Record: DECLTYPE();
  case Decl::CXXRecord: return true;
  case Decl::ClassTemplateSpecialization: return true;
  case Decl::ClassTemplatePartialSpecialization: return true;
  case Decl::TemplateTypeParm: return true;
  case Decl::TypeAlias: return true;
  case Decl::Typedef: DECLTYPE();
  case Decl::UnresolvedUsingTypename: return true;
  case Decl::Using: return true;
  case Decl::UsingDirective: return true;
  case Decl::UsingShadow: return true;
  case Decl::Field: return true;
  case Decl::ObjCAtDefsField: return true;
  case Decl::ObjCIvar: return true;
  case Decl::Function: DECLTYPE();
  case Decl::CXXMethod: return true;
  case Decl::CXXConstructor: return true;
  case Decl::CXXConversion: return true;
  case Decl::CXXDestructor: return true;
  case Decl::MSProperty: return true;
  case Decl::NonTypeTemplateParm: return true;
  case Decl::Var: DECLTYPE();
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

///
/// Local Variables:
/// indent-tabs-mode: nil
/// c-basic-offset: 2
/// End:
///
