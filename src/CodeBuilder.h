#ifndef CODEBUILDER_H
#define CODEBUILDER_H

class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  TypeMap typeTable;
};

using CodeBuilder = Reality<const SourceInfo&, std::stringstream&>;

void buildCode(xmlDocPtr, std::stringstream&);

#endif /* !CODEBUILDER_H */
