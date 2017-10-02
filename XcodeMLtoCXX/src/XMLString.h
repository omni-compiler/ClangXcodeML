#ifndef XMLSTRING_H
#define XMLSTRING_H

/*!
 * \brief A wrapper of \c xmlChar*.
 */
class XMLString {
public:
  XMLString(const xmlChar *);
  XMLString(const char *);
  const xmlChar *c_ptr() const;
  operator std::string() const;

private:
  std::string str;
};

XMLString operator+(const XMLString lhs, const XMLString rhs);
bool operator==(const XMLString lhs, const XMLString rhs);
std::ostream &operator<<(std::ostream &os, const XMLString &str);
size_t length(XMLString str);

#endif /* !XMLSTRING_H */
