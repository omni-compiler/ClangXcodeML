#ifndef SOURCEINFO_H
#define SOURCEINFO_H

/*!
 * \brief A pack of necessary information for generating
 * C++ source code.
 */
class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  XcodeMl::Environment typeTable;
  /*! SymbolEntry stack in current scope. */
  SymbolMap symTable;
  unsigned int indentation;
};

#endif /* !SOURCEINFO_H */
