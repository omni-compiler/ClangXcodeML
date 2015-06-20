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

bool
TypeTableVisitor::PreVisitDecl(Decl *D) {
  if (!D) {
    return false;
  }
  if (D->getKind() == Decl::TranslationUnit) {
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
