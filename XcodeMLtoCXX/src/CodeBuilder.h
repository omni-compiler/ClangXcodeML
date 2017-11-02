#ifndef CODEBUILDER_H
#define CODEBUILDER_H

using CodeBuilder = XMLWalker<CXXCodeGen::StringTreeRef, SourceInfo &>;

extern CodeBuilder const ProgramBuilder;

void buildCode(xmlNodePtr, xmlXPathContextPtr, std::stringstream &);

#endif /* !CODEBUILDER_H */
