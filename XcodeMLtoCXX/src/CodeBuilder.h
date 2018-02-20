#ifndef CODEBUILDER_H
#define CODEBUILDER_H

using CodeBuilder = XMLWalker<CXXCodeGen::StringTreeRef, SourceInfo &>;

extern CodeBuilder const ProgramBuilder;

extern CodeBuilder const ClassDefinitionBuilder;

XcodeMl::CodeFragment declareClassTypeInit(
    const CodeBuilder &, xmlNodePtr ctorExpr, SourceInfo &src);

void buildCode(xmlNodePtr, xmlXPathContextPtr, std::stringstream &);

#endif /* !CODEBUILDER_H */
