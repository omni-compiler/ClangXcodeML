#include <libxml/xmlreader.h>
#include <string>
#include <iostream>
#include <vector>

void processNode(xmlTextReaderPtr reader);

int main(int argc, char **argv) {
  std::string filename(argv[1]);
  xmlTextReaderPtr reader = xmlNewTextReaderFilename(filename.c_str());
  while (xmlTextReaderRead(reader) == 1) {
    processNode(reader);
  }
  xmlFreeTextReader(reader);
  return 0;
}

std::vector<xmlChar*> path;

void processNode(xmlTextReaderPtr reader) {
  int nodeType = xmlTextReaderNodeType(reader);
  xmlChar* name = xmlTextReaderName(reader);
  if (!name) {
    name = xmlStrdup(BAD_CAST "---");
  }
  std::cout << nodeType << ":\t";
  for (xmlChar* str : path) {
    std::cout << str << ">";
  }
  std::cout << std::endl;
  if (nodeType == XML_READER_TYPE_ELEMENT) {
    path.push_back(name);
  } else if (nodeType == XML_READER_TYPE_END_ELEMENT) {
    path.pop_back();
  } else if (nodeType == XML_READER_TYPE_TEXT) {
    xmlChar* value = xmlTextReaderValue(reader);
    if (!value) {
      value = xmlStrdup(BAD_CAST "---");
    }
    for (int i = 1, limit = path.size(); i < limit; ++i) {
      std::cout << '.';
    }
    std::cout << value << std::endl;
  }
}
