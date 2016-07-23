#ifndef CODEBUILDER_H
#define CODEBUILDER_H

/*!
 * \brief Mapping from variable names(variables, constants, etc.)
 * to data type identifiers declared in a single scope.
 */
using SymbolEntry = std::map<std::string,std::string>;
/*!
 * \brief A stack of SymbolEntry.
 *
 * It contains all visible names in a scope.
 */
using SymbolMap = std::vector<SymbolEntry>;

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
};

void buildCode(xmlDocPtr, std::stringstream&);

#endif /* !CODEBUILDER_H */
