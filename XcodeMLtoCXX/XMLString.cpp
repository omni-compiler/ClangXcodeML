#include <libxml/xmlstring.h>
#include <string>
#include <sstream>
#include "XMLString.h"

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

