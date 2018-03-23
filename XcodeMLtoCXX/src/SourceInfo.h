#ifndef SOURCEINFO_H
#define SOURCEINFO_H

namespace XcodeMl {
class TypeTable;
} // namespace XcodeMl

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
  explicit SourceInfo(xmlXPathContextPtr c,
      const XcodeMl::TypeTable &e,
      const XcodeMl::NnsMap &n,
      Language l);
  std::string getUniqueName();

  xmlXPathContextPtr ctxt;
  XcodeMl::TypeTable typeTable;
  XcodeMl::NnsMap nnsTable;
  Language language;

private:
  size_t uniqueNameIndex;
};

#endif /* !SOURCEINFO_H */
