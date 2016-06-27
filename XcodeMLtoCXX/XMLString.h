#ifndef XMLSTRING_H
#define XMLSTRING_H

class XMLString {
public:
  XMLString(const xmlChar*);
  XMLString(const char*);
  const xmlChar* c_ptr() const;
  operator std::string() {
    std::stringstream ss;
    ss << c_ptr();
    return ss.str();
  }
private:
  const xmlChar* ptr;
};

XMLString operator+(const XMLString lhs, const XMLString rhs);
bool operator==(const XMLString lhs, const XMLString rhs);
std::ostream& operator<<(std::ostream& os, const XMLString& str);
size_t length(XMLString str);

#endif /* !XMLSTRING_H */
