#include <stdio.h>
#include <clang-c/Index.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

xmlChar *getLocationAsString(CXCursor cursor)
{
  xmlChar locStr[BUFSIZ];
  xmlChar *ret = NULL;

  // get filename, line # and column #.
  CXSourceLocation loc = clang_getCursorLocation(cursor);
  CXString filename;
  unsigned lineno;
  unsigned column;
  clang_getPresumedLocation(loc, &filename, &lineno, &column);

  xmlStrPrintf(locStr, BUFSIZ-1,
               BAD_CAST "%s:%d:%d",
               clang_getCString(filename),
               lineno,
               column);
  ret = xmlStrdup(locStr);

  clang_disposeString(filename);

  return ret;
}

CXChildVisitResult visitChildrenCallback(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  xmlTextWriterPtr xmlWriter = static_cast<xmlTextWriterPtr>(client_data);
  enum CXCursorKind kind = clang_getCursorKind(cursor);
  CXString kindStr = clang_getCursorKindSpelling(kind);
  xmlChar *kindname = BAD_CAST clang_getCString (kindStr);

  switch (kind) {
  case CXCursor_UnexposedDecl: break;
  case CXCursor_StructDecl: kindname = BAD_CAST "structType"; break;
  case CXCursor_UnionDecl: break;
  case CXCursor_ClassDecl: break;
  case CXCursor_EnumDecl: break;
  case CXCursor_FieldDecl: break;
  case CXCursor_EnumConstantDecl: break;
  case CXCursor_FunctionDecl: kindname = BAD_CAST "functionDefinition"; break;
  case CXCursor_VarDecl: kindname = BAD_CAST "varDecl"; break;
  case CXCursor_ParmDecl: break;
  case CXCursor_ObjCInterfaceDecl: break;
  case CXCursor_ObjCCategoryDecl: break;
  case CXCursor_ObjCProtocolDecl: break;
  case CXCursor_ObjCPropertyDecl: break;
  case CXCursor_ObjCIvarDecl: break;
  case CXCursor_ObjCInstanceMethodDecl: break;
  case CXCursor_ObjCClassMethodDecl: break;
  case CXCursor_ObjCImplementationDecl: break;
  case CXCursor_ObjCCategoryImplDecl: break;
  case CXCursor_TypedefDecl: break;
  case CXCursor_CXXMethod: break;
  case CXCursor_Namespace: break;
  case CXCursor_LinkageSpec: break;
  case CXCursor_Constructor: break;
  case CXCursor_Destructor: break;
  case CXCursor_ConversionFunction: break;
  case CXCursor_TemplateTypeParameter: break;
  case CXCursor_NonTypeTemplateParameter: break;
  case CXCursor_TemplateTemplateParameter: break;
  case CXCursor_FunctionTemplate: break;
  case CXCursor_ClassTemplate: break;
  case CXCursor_ClassTemplatePartialSpecialization: break;
  case CXCursor_NamespaceAlias: break;
  case CXCursor_UsingDirective: break;
  case CXCursor_UsingDeclaration: break;
  case CXCursor_TypeAliasDecl: break;
  case CXCursor_ObjCSynthesizeDecl: break;
  case CXCursor_ObjCDynamicDecl: break;
  case CXCursor_CXXAccessSpecifier: break;
    //case CXCursor_FirstDecl:
    //case CXCursor_LastDecl:
  case CXCursor_ObjCSuperClassRef: break;
  case CXCursor_ObjCProtocolRef: break;
  case CXCursor_ObjCClassRef: break;
  case CXCursor_TypeRef: break;
  case CXCursor_CXXBaseSpecifier: break;
  case CXCursor_TemplateRef: break;
  case CXCursor_NamespaceRef: break;
  case CXCursor_MemberRef: break;
  case CXCursor_LabelRef: break;
  case CXCursor_OverloadedDeclRef: break;
  case CXCursor_VariableRef: break;
    //case CXCursor_LastRef:
    //case CXCursor_FirstInvalid:
  case CXCursor_InvalidFile: break;
  case CXCursor_NoDeclFound: break;
  case CXCursor_NotImplemented: break;
  case CXCursor_InvalidCode: break;
    //case CXCursor_LastInvalid:
    //case CXCursor_FirstExpr:
  case CXCursor_UnexposedExpr: break;
  case CXCursor_DeclRefExpr: break;
  case CXCursor_MemberRefExpr: break;
  case CXCursor_CallExpr: break;
  case CXCursor_ObjCMessageExpr: break;
  case CXCursor_BlockExpr: break;
  case CXCursor_IntegerLiteral: break;
  case CXCursor_FloatingLiteral: break;
  case CXCursor_ImaginaryLiteral: break;
  case CXCursor_StringLiteral: break;
  case CXCursor_CharacterLiteral: break;
  case CXCursor_ParenExpr: break;
  case CXCursor_UnaryOperator: break;
  case CXCursor_ArraySubscriptExpr: break;
  case CXCursor_BinaryOperator: break;
  case CXCursor_CompoundAssignOperator: break;
  case CXCursor_ConditionalOperator: break;
  case CXCursor_CStyleCastExpr: break;
  case CXCursor_CompoundLiteralExpr: break;
  case CXCursor_InitListExpr: break;
  case CXCursor_AddrLabelExpr: break;
  case CXCursor_StmtExpr: break;
  case CXCursor_GenericSelectionExpr: break;
  case CXCursor_GNUNullExpr: break;
  case CXCursor_CXXStaticCastExpr: break;
  case CXCursor_CXXDynamicCastExpr: break;
  case CXCursor_CXXReinterpretCastExpr: break;
  case CXCursor_CXXConstCastExpr: break;
  case CXCursor_CXXFunctionalCastExpr: break;
  case CXCursor_CXXTypeidExpr: break;
  case CXCursor_CXXBoolLiteralExpr: break;
  case CXCursor_CXXNullPtrLiteralExpr: break;
  case CXCursor_CXXThisExpr: break;
  case CXCursor_CXXThrowExpr: break;
  case CXCursor_CXXNewExpr: break;
  case CXCursor_CXXDeleteExpr: break;
  case CXCursor_UnaryExpr: break;
  case CXCursor_ObjCStringLiteral: break;
  case CXCursor_ObjCEncodeExpr: break;
  case CXCursor_ObjCSelectorExpr: break;
  case CXCursor_ObjCProtocolExpr: break;
  case CXCursor_ObjCBridgedCastExpr: break;
  case CXCursor_PackExpansionExpr: break;
  case CXCursor_SizeOfPackExpr: break;
  case CXCursor_LambdaExpr: break;
  case CXCursor_ObjCBoolLiteralExpr: break;
  case CXCursor_ObjCSelfExpr: break;
    //case CXCursor_LastExpr:
    //case CXCursor_FirstStmt:
  case CXCursor_UnexposedStmt: break;
  case CXCursor_LabelStmt: break;
  case CXCursor_CompoundStmt: kindname = BAD_CAST "compoundStatement"; break;
  case CXCursor_CaseStmt: break;
  case CXCursor_DefaultStmt: break;
  case CXCursor_IfStmt: break;
  case CXCursor_SwitchStmt: break;
  case CXCursor_WhileStmt: break;
  case CXCursor_DoStmt: break;
  case CXCursor_ForStmt: break;
  case CXCursor_GotoStmt: break;
  case CXCursor_IndirectGotoStmt: break;
  case CXCursor_ContinueStmt: break;
  case CXCursor_BreakStmt: break;
  case CXCursor_ReturnStmt: break;
    //case CXCursor_GCCAsmStmt:
  case CXCursor_AsmStmt: break;
  case CXCursor_ObjCAtTryStmt: break;
  case CXCursor_ObjCAtCatchStmt: break;
  case CXCursor_ObjCAtFinallyStmt: break;
  case CXCursor_ObjCAtThrowStmt: break;
  case CXCursor_ObjCAtSynchronizedStmt: break;
  case CXCursor_ObjCAutoreleasePoolStmt: break;
  case CXCursor_ObjCForCollectionStmt: break;
  case CXCursor_CXXCatchStmt: break;
  case CXCursor_CXXTryStmt: break;
  case CXCursor_CXXForRangeStmt: break;
  case CXCursor_SEHTryStmt: break;
  case CXCursor_SEHExceptStmt: break;
  case CXCursor_SEHFinallyStmt: break;
  case CXCursor_MSAsmStmt: break;
  case CXCursor_NullStmt: break;
  case CXCursor_DeclStmt: break;
  case CXCursor_OMPParallelDirective: break;
  case CXCursor_OMPSimdDirective: break;
  case CXCursor_OMPForDirective: break;
  case CXCursor_OMPSectionsDirective: break;
  case CXCursor_OMPSectionDirective: break;
  case CXCursor_OMPSingleDirective: break;
  case CXCursor_OMPParallelForDirective: break;
  case CXCursor_OMPParallelSectionsDirective: break;
  case CXCursor_OMPTaskDirective: break;
  case CXCursor_OMPMasterDirective: break;
  case CXCursor_OMPCriticalDirective: break;
  case CXCursor_OMPTaskyieldDirective: break;
  case CXCursor_OMPBarrierDirective: break;
  case CXCursor_OMPTaskwaitDirective: break;
  case CXCursor_OMPFlushDirective: break;
  case CXCursor_SEHLeaveStmt: break;
#if 0
  case CXCursor_OMPOrderedDirective:
  case CXCursor_OMPAtomicDirective:
  case CXCursor_OMPForSimdDirective:
  case CXCursor_OMPParallelForSimdDirective:
  case CXCursor_OMPTargetDirective:
  case CXCursor_OMPTeamsDirective:
#endif
    //case CXCursor_LastStmt:
  case CXCursor_TranslationUnit: break;
    //case CXCursor_FirstAttr:
  case CXCursor_UnexposedAttr: break;
  case CXCursor_IBActionAttr: break;
  case CXCursor_IBOutletAttr: break;
  case CXCursor_IBOutletCollectionAttr: break;
  case CXCursor_CXXFinalAttr: break;
  case CXCursor_CXXOverrideAttr: break;
  case CXCursor_AnnotateAttr: break;
  case CXCursor_AsmLabelAttr: break;
  case CXCursor_PackedAttr: break;
  case CXCursor_PureAttr: break;
  case CXCursor_ConstAttr: break;
  case CXCursor_NoDuplicateAttr: break;
  case CXCursor_CUDAConstantAttr: break;
  case CXCursor_CUDADeviceAttr: break;
  case CXCursor_CUDAGlobalAttr: break;
  case CXCursor_CUDAHostAttr: break;
#if 0
  case CXCursor_CUDASharedAttr:
#endif
    //case CXCursor_LastAttr:
  case CXCursor_PreprocessingDirective: break;
  case CXCursor_MacroDefinition: break;
  case CXCursor_MacroExpansion: break;
    //case CXCursor_MacroInstantiation:
  case CXCursor_InclusionDirective: break;
    //case CXCursor_FirstPreprocessing:
    //case CXCursor_LastPreprocessing:
  case CXCursor_ModuleImportDecl: break;
    //case CXCursor_FirstExtraDecl:
    //case CXCursor_LastExtraDecl:
#if 0
  case CXCursor_OverloadCandidate: break;
#endif
  }

  xmlTextWriterStartElement(xmlWriter, kindname);
  clang_disposeString(kindStr);

  xmlChar *location = getLocationAsString(cursor);
  CXString displayNameStr = clang_getCursorDisplayName(cursor);
  xmlTextWriterWriteAttribute(xmlWriter,
                              BAD_CAST "src",
                              location);
  xmlTextWriterWriteAttribute(xmlWriter,
                              BAD_CAST "displayname",
                              BAD_CAST clang_getCString(displayNameStr));
  clang_disposeString(displayNameStr);
  free(location);

#if 0
  CXString usrStr = clang_getCursorUSR(cursor);

  xmlTextWriterWriteAttribute(xmlWriter,
                              BAD_CAST "usr",
                              BAD_CAST clang_getCString(usrStr));
  clang_disposeString(usrStr);
#endif

 end:
  // visit children recursively.
  clang_visitChildren(cursor,
                      visitChildrenCallback,
                      xmlWriter);
  // emit a closing tag corresponds to the header emitted above
  xmlTextWriterEndElement(xmlWriter);

  return CXChildVisit_Continue;
}

int main(int argc, char *argv[])
{
  // create index w/ excludeDeclsFromPCH = 1, displayDiagnostics=1.
  CXIndex index = clang_createIndex(1 /* excludeDeclarationFromPCH */,
				    1 /* displayDiagnostics */);

  // load a *.ast file.
  CXTranslationUnit tu = clang_createTranslationUnit(index, argv[1]);
  if (tu != NULL) {
    //xmlBufferPtr xmlBuffer = xmlBufferCreate();
    xmlDocPtr xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    xmlTextWriterPtr xmlWriter = xmlNewTextWriterDoc(&xmlDoc, 0);

    xmlTextWriterStartDocument(xmlWriter, NULL, "UTF-8", NULL);
    xmlTextWriterStartElement(xmlWriter, BAD_CAST "XcodeProgram");

    clang_visitChildren(clang_getTranslationUnitCursor(tu),
                        visitChildrenCallback,
                        xmlWriter);

    xmlTextWriterEndElement(xmlWriter); // XcodeProgram
    xmlTextWriterEndDocument(xmlWriter);
    xmlFreeTextWriter(xmlWriter);

    // print the XML text to the standard output.
    xmlDocFormatDump(stdout, xmlDoc, 1);
    xmlFreeDoc(xmlDoc);

    clang_disposeTranslationUnit(tu);
  } else {
    fprintf(stderr, "Could not load \"%s\" as an AST file.\n", argv[1]);
  }

  clang_disposeIndex(index);

  return 0;
}
