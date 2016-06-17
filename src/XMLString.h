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

XMLString::XMLString(const xmlChar * p) : ptr(p) {}

XMLString::XMLString(const char *str) : ptr(BAD_CAST str) {}

const xmlChar* XMLString::c_ptr() const {
  return ptr;
}

XMLString operator+(const XMLString lhs, const XMLString rhs) {
  xmlChar* dst = xmlStrdup(lhs.c_ptr());
  return xmlStrcat(dst, rhs.c_ptr());
}

bool operator==(const XMLString lhs, const XMLString rhs) {
  return xmlStrEqual(lhs.c_ptr(), rhs.c_ptr());
}

size_t length(XMLString str) {
  return static_cast<size_t>(xmlStrlen(str.c_ptr()));
}

