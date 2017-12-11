#ifndef SOURCEINFO_H
#define SOURCEINFO_H

enum class Language {
  Invalid,
  C,
  CPlusPlus,
};

/*!
 * \brief A pack of necessary information for generating
 * C++ source code.
 */
class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  XcodeMl::Environment typeTable;
  XcodeMl::NnsMap nnsTable;
  Language language;
  std::map<std::string, CXXCodeGen::StringTreeRef> unnamedClassDecls;
};

#endif /* !SOURCEINFO_H */
