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
  std::string getUniqueName();

  xmlXPathContextPtr ctxt;
  XcodeMl::Environment typeTable;
  XcodeMl::NnsMap nnsTable;
  Language language;

private:
  size_t uniqueNameIndex;
};

#endif /* !SOURCEINFO_H */
