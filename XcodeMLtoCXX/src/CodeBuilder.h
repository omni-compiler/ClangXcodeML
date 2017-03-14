#ifndef CODEBUILDER_H
#define CODEBUILDER_H

using CodeBuilder = XMLWalker<void, SourceInfo&, CXXCodeGen::Stream&>;

void buildCode(xmlNodePtr, xmlXPathContextPtr, std::stringstream&);

#endif /* !CODEBUILDER_H */
