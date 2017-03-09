#ifndef XCODEMLENVIRONMENT_H
#define XCODEMLENVIRONMENT_H

namespace XcodeMl {

  /*!
   * \brief A mapping from data type identifiers
   * to actual data types.
   */
  class Environment {
  public:
    using ReturnType = TypeRef;
  public:
    const TypeRef& operator[](const std::string&) const;
    TypeRef& operator[](const std::string&);
    const TypeRef& at(const std::string&) const;
    TypeRef& at(const std::string&);
    const ReturnType& getReturnType(const std::string&) const;
    void setReturnType(const std::string&, const TypeRef&);
    const std::vector<std::string>& getKeys(void) const;
  private:
    using TypeMap = std::map<std::string, TypeRef>;
    TypeRef& at_or_throw(TypeMap&, const std::string&, const std::string&)
      const;
    const TypeRef& at_or_throw(const TypeMap&, const std::string&, const std::string&) const;
    std::map<std::string, TypeRef> map;
    std::map<std::string, ReturnType> returnMap;
    std::vector<std::string> keys;
  };

}

#endif /* XCODEMLENVIRONMENT_H */
