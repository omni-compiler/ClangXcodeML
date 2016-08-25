#include <stdio.h>
#include <clang-c/Index.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <time.h>
#include <getopt.h>

int opt_debug = 0;
int opt_location = 0;

typedef struct {
  xmlNodePtr curNode;
  xmlNodePtr curBlock;
  xmlNodePtr typeTable;
  xmlNodePtr globalSymbols;
  xmlNodePtr globalDeclarations;
  xmlDocPtr xmlDoc;
} MyClientData;

void insertLocationInfo(CXCursor cursor, xmlNodePtr node,
			xmlChar *prop_filename,
			xmlChar *prop_lineno,
			xmlChar *prop_column,
			xmlChar *prop_range)
{
  CXSourceLocation loc = clang_getCursorLocation(cursor);
  CXSourceRange range = clang_getCursorExtent(cursor);
  CXString filename;
  unsigned lineno;
  unsigned column;

  // location
  clang_getPresumedLocation(loc, &filename, &lineno, &column);
  if (prop_column != NULL) {
    xmlChar columnStr[BUFSIZ];
    xmlStrPrintf(columnStr, BUFSIZ, BAD_CAST "%d", column);
    xmlNewProp(node, prop_column, columnStr);
  }
  if (prop_lineno != NULL) {
    xmlChar linenoStr[BUFSIZ];
    xmlStrPrintf(linenoStr, BUFSIZ, BAD_CAST "%d", lineno);
    xmlNewProp(node, prop_lineno, linenoStr);
  }
  if (prop_filename != NULL) {
    xmlNewProp(node, prop_filename, BAD_CAST clang_getCString(filename));
  }

  // range
  if (prop_range != NULL && !clang_Range_isNull(range)) {
    xmlChar rangeStr[BUFSIZ];

    CXSourceLocation start = clang_getRangeStart(range);
    CXSourceLocation end = clang_getRangeEnd(range);
    CXString start_filename;
    unsigned start_lineno;
    unsigned start_column;
    CXString end_filename;
    unsigned end_lineno;
    unsigned end_column;
    xmlChar *filenameStr;
    xmlChar *start_filenameStr;
    xmlChar *end_filenameStr;

    clang_getPresumedLocation(start, &start_filename, &start_lineno, &start_column);
    clang_getPresumedLocation(end, &end_filename, &end_lineno, &end_column);

    filenameStr = BAD_CAST clang_getCString(filename);
    start_filenameStr = BAD_CAST clang_getCString(start_filename);
    end_filenameStr = BAD_CAST clang_getCString(end_filename);

    if (xmlStrcmp(start_filenameStr, filenameStr) != 0
	|| xmlStrcmp(end_filenameStr, filenameStr) != 0) {
      xmlStrPrintf(rangeStr, BUFSIZ, BAD_CAST "%s:%d:%d - %s:%d:%d",
		   start_filenameStr, start_lineno, start_column,
		   end_filenameStr, end_lineno, end_column);
    } else if (start_lineno != end_lineno) {
      xmlStrPrintf(rangeStr, BUFSIZ, BAD_CAST "%d:%d - %d:%d",
		   start_lineno, start_column,
		   end_lineno, end_column);
    } else {
      xmlStrPrintf(rangeStr, BUFSIZ, BAD_CAST "%d:%d-%d",
		   start_lineno, start_column, end_column);
    }
    clang_disposeString(end_filename);
    clang_disposeString(start_filename);
    xmlNewProp(node, prop_range, rangeStr);
  }
  clang_disposeString(filename);
}

void insertCXCursorInfo(xmlNodePtr parentnode, CXCursor cursor)
{
  xmlChar commentStr[BUFSIZ];
  int offset = 0;

  enum CXCursorKind kind = clang_getCursorKind(cursor);
  CXString kindSpelling = clang_getCursorKindSpelling(kind);
  xmlChar *kindSpellingStr = BAD_CAST clang_getCString(kindSpelling);

  offset += xmlStrPrintf(commentStr + offset, BUFSIZ - offset,
			 BAD_CAST " kind=%s", kindSpellingStr);
  clang_disposeString(kindSpelling);

  CXString displayName = clang_getCursorDisplayName(cursor);
  xmlChar *displayNameStr = BAD_CAST clang_getCString(displayName);
  if (xmlStrlen(displayNameStr) > 0) {
    offset += xmlStrPrintf(commentStr + offset, BUFSIZ - offset,
			   BAD_CAST ", displayName=%s", displayNameStr); 
  }
  clang_disposeString(displayName);

  CXString usr = clang_getCursorUSR(cursor);
  xmlChar *usrStr = BAD_CAST clang_getCString(usr);
  if (xmlStrlen(usrStr) > 0) {
    offset += xmlStrPrintf(commentStr + offset, BUFSIZ - offset,
			   BAD_CAST ", USR=%s", usrStr);
  }
  clang_disposeString(usr);

  CXString spelling = clang_getCursorSpelling(cursor);
  xmlChar *spellingStr = BAD_CAST clang_getCString(spelling);
  if (xmlStrlen(spellingStr) > 0) {
    offset += xmlStrPrintf(commentStr + offset, BUFSIZ - offset,
			   BAD_CAST ", spelling=%s", spellingStr);
  }
  clang_disposeString(spelling);

  xmlStrPrintf(commentStr + offset, BUFSIZ - offset, BAD_CAST " ");

  xmlAddChild(parentnode, xmlNewComment(commentStr));
}

CXChildVisitResult visitChildrenCallback(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  MyClientData *parentclientdata = static_cast<MyClientData*>(client_data);
  MyClientData myclientdata = *parentclientdata;
  enum CXCursorKind kind = clang_getCursorKind(cursor);
  CXString kindSpelling = clang_getCursorKindSpelling(kind);
  xmlChar *kindStr = BAD_CAST clang_getCString(kindSpelling);
  xmlNodePtr parentnode = parentclientdata->curNode;

  if (opt_debug) {
    insertCXCursorInfo(parentnode, cursor);
  }

  switch (kind) {
  case CXCursor_UnexposedDecl: break;
  case CXCursor_StructDecl:
    kindStr = BAD_CAST "structType";
    parentnode = myclientdata.typeTable;
    break;
  case CXCursor_UnionDecl: break;
    kindStr = BAD_CAST "unionType";
    parentnode = myclientdata.typeTable;
    break;
  case CXCursor_ClassDecl: break;
  case CXCursor_EnumDecl: break;
    kindStr = BAD_CAST "enumType";
    parentnode = myclientdata.typeTable;
    break;
  case CXCursor_FieldDecl: break;
  case CXCursor_EnumConstantDecl: break;
  case CXCursor_FunctionDecl:
    kindStr = BAD_CAST "functionDefinition";
    break;
  case CXCursor_VarDecl:
    kindStr = BAD_CAST "varDecl";
    parentnode = myclientdata.typeTable;
    break;
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
  case CXCursor_UnexposedExpr:
    kindStr = NULL;
    break;
  case CXCursor_DeclRefExpr:
    kindStr = BAD_CAST "Var";
    break;
  case CXCursor_MemberRefExpr: break;
  case CXCursor_CallExpr: break;
  case CXCursor_ObjCMessageExpr: break;
  case CXCursor_BlockExpr: break;
  case CXCursor_IntegerLiteral:
    kindStr = BAD_CAST "intConstant";
    parentnode = xmlNewChild(parentnode, NULL, BAD_CAST "value", NULL);
    
    break;
  case CXCursor_FloatingLiteral:
    kindStr = BAD_CAST "floatConstant";
    parentnode = xmlNewChild(parentnode, NULL, BAD_CAST "value", NULL);
    break;
  case CXCursor_ImaginaryLiteral: break;
  case CXCursor_StringLiteral:
    kindStr = BAD_CAST "stringConstant";
    parentnode = xmlNewChild(parentnode, NULL, BAD_CAST "value", NULL);
    break;
  case CXCursor_CharacterLiteral: break;
  case CXCursor_ParenExpr: break;
  case CXCursor_UnaryOperator: break;
  case CXCursor_ArraySubscriptExpr: break;
  case CXCursor_BinaryOperator: break;
  case CXCursor_CompoundAssignOperator: break;
  case CXCursor_ConditionalOperator: break;
  case CXCursor_CStyleCastExpr: break;
  case CXCursor_CompoundLiteralExpr: break;
  case CXCursor_InitListExpr:
    kindStr = BAD_CAST "value";
    break;
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
  case CXCursor_CompoundStmt:
    kindStr = BAD_CAST "compoundStatement";
    myclientdata.curBlock = myclientdata.curNode;
    break;
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

  myclientdata = *parentclientdata;
  if (kindStr != NULL) {
    xmlNodePtr node = xmlNewChild(parentnode, NULL, kindStr, NULL);

    if (opt_location) {
      xmlChar *fileStr = NULL;
      xmlChar *linenoStr = NULL;
      xmlChar *columnStr = NULL;
      xmlChar *rangeStr = NULL;
      if (opt_location >= 1) {
	linenoStr = BAD_CAST "lineno";
      }
      if (opt_location >= 2) {
	fileStr = BAD_CAST "file";
      }
      if (opt_location >= 3) {
	columnStr = BAD_CAST "column";
      }
      if (opt_location >= 4) {
	rangeStr = BAD_CAST "range";
      }
      insertLocationInfo(cursor, node, fileStr, linenoStr, columnStr, rangeStr);
    }

    CXType typ = clang_getCursorType(cursor);
    CXString typSpelling = clang_getTypeSpelling(typ);
    xmlChar *typSpellingStr = BAD_CAST clang_getCString(typSpelling);

    xmlNewProp(node, BAD_CAST "type", typSpellingStr);

    myclientdata.curNode = node;
  }
  clang_disposeString(kindSpelling);

  // visit children recursively.
  clang_visitChildren(cursor, visitChildrenCallback, &myclientdata);

  return CXChildVisit_Continue;
}

int main(int argc, char *argv[])
{
  int opt;

  while ((opt = getopt(argc, argv, "dl")) != -1) {
    switch (opt) {
    case 'd':
      opt_debug++;
      break;
    case 'l':
      opt_location++;
      break;
    default:
      fprintf(stderr,
	      "unknown option: -%c\n"
	      "Usage: %s [-dl] <filename.ast>\n"
	      "\t-d: output debugging info\n"
	      "\t-l: output source-code location info\n",
	      opt, argv[0]);
      return 1;
    }
  }
  if (optind >= argc) {
      fprintf(stderr,
	      "Missing input filename.\n"
	      "Usage: %s [options] <filename.ast>\n",
	      argv[0]);
      return 1;
  }
  argv += optind;
  argc -= optind;

  // create index w/ excludeDeclsFromPCH = 1, displayDiagnostics=1.
  CXIndex index = clang_createIndex(1 /* excludeDeclarationFromPCH */,
				    1 /* displayDiagnostics */);
  MyClientData myclientdata = {NULL, NULL, NULL, NULL};

  // load a *.ast file.
  CXTranslationUnit tu = clang_createTranslationUnit(index, argv[0]);
  if (tu != NULL) {
    //xmlBufferPtr xmlBuffer = xmlBufferCreate();
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr rootnode = xmlNewNode(NULL, BAD_CAST "XcodeProgram");
    char strftimebuf[BUFSIZ];
    time_t t = time(NULL);

    strftime(strftimebuf, sizeof strftimebuf, "%F %T", localtime(&t));

    CXString tuSpelling = clang_getTranslationUnitSpelling(tu);
    xmlNewProp(rootnode, BAD_CAST "source", BAD_CAST clang_getCString(tuSpelling));
    clang_disposeString(tuSpelling);
    xmlNewProp(rootnode, BAD_CAST "language", BAD_CAST "C");
	     xmlNewProp(rootnode, BAD_CAST "time", BAD_CAST strftimebuf);

    xmlDocSetRootElement(doc, rootnode);
    myclientdata.curNode = rootnode;
    myclientdata.curBlock = rootnode;
    myclientdata.typeTable
      = xmlNewChild(rootnode, NULL, BAD_CAST "typeTable", NULL);
    myclientdata.globalSymbols
      = xmlNewChild(rootnode, NULL, BAD_CAST "globalSymbols", NULL);
    myclientdata.globalDeclarations
      = xmlNewChild(rootnode, NULL, BAD_CAST "globalDeclarations", NULL);
    myclientdata.xmlDoc = doc;
    clang_visitChildren(clang_getTranslationUnitCursor(tu),
			visitChildrenCallback, &myclientdata);

    // print the XML text to the standard output.
    xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
    xmlFreeDoc(doc);

    clang_disposeTranslationUnit(tu);
  } else {
    fprintf(stderr, "Could not load \"%s\" as an AST file.\n", argv[1]);
  }

  clang_disposeIndex(index);

  return 0;
}
