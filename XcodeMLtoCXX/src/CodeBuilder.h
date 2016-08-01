#ifndef CODEBUILDER_H
#define CODEBUILDER_H

/*!
 * \brief A pack of necessary information for generating
 * C++ source code.
 */
class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  TypeMap typeTable;
  /*! SymbolEntry stack in current scope. */
  SymbolMap symTable;
  unsigned int indentation;
};

void buildCode(xmlDocPtr, std::stringstream&);

#endif /* !CODEBUILDER_H */
