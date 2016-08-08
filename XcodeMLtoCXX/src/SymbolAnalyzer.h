#ifndef SYMBOLANALYZER_H
#define SYMBOLANALYZER_H

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

#endif /* !SYMBOLANALYZER_H */
