#ifndef CODEBUILDER_H
#define CODEBUILDER_H

using SymbolEntry = std::map<std::string,std::string>;
using SymbolMap = std::vector<SymbolEntry>;

class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  XcodeMl::TypeMap typeTable;
  SymbolMap symTable;
};

using CodeBuilder = Reality<SourceInfo&, std::stringstream&>;

void buildCode(xmlDocPtr, std::stringstream&);

#endif /* !CODEBUILDER_H */
