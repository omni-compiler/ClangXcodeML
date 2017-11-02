#ifndef CODEBUILDER_H
#define CODEBUILDER_H

using CodeBuilder = XMLWalker<CXXCodeGen::StringTreeRef, SourceInfo &>;

extern CodeBuilder const ProgramBuilder;

extern CodeBuilder const ClassDefinitionBuilder;

void buildCode(xmlNodePtr, xmlXPathContextPtr, std::stringstream &);

#endif /* !CODEBUILDER_H */
