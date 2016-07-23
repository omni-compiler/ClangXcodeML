#ifndef TYPEANALYZER_H
#define TYPEANALYZER_H

/*!
 * \brief A mapping from data type identifiers
 * to actual data types.
 */
class TypeMap {
public:
  const XcodeMl::TypeRef& operator[](const std::string&) const;
  XcodeMl::TypeRef& operator[](const std::string&);
  const XcodeMl::TypeRef& getReturnType(const std::string&) const;
  void setReturnType(const std::string&, const XcodeMl::TypeRef&);
private:
  std::map<std::string, XcodeMl::TypeRef> map;
  std::map<std::string, XcodeMl::TypeRef> returnMap;
};

TypeMap parseTypeTable(xmlDocPtr doc);

#endif /* !TYPEANALYZER_H */
