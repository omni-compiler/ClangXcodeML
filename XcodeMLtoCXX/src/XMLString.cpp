#include <libxml/xmlstring.h>
#include <string>
#include <sstream>
#include "XMLString.h"

XMLString::XMLString(const xmlChar *p) {
  std::stringstream ss;
  ss << p;
  str = ss.str();
}

XMLString::XMLString(const char *s) : str(s) {
}

const xmlChar *
XMLString::c_ptr() const {
  return BAD_CAST str.c_str();
}

XMLString::operator std::string() const {
  return str;
}

XMLString operator+(const XMLString lhs, const XMLString rhs) {
  xmlChar *dst = xmlStrdup(lhs.c_ptr());
  return xmlStrcat(dst, rhs.c_ptr());
}

bool operator==(const XMLString lhs, const XMLString rhs) {
  return static_cast<std::string>(lhs) == static_cast<std::string>(rhs);
}

size_t
length(XMLString str) {
  return static_cast<std::string>(str).length();
}

std::ostream &operator<<(std::ostream &os, const XMLString &str) {
  os << str.c_ptr();
  return os;
}
