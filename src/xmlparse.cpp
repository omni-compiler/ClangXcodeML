#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <sstream>
#include <cassert>
#include <memory>
#include <vector>
#include "XMLString.h"
#include "XcodeMlType.h"
#include "Reality.h"
#include "CodeBuilder.h"
#include "TypeAnalyzer.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
    return 0;
  }
  std::string filename(argv[1]);
  xmlDocPtr doc = xmlParseFile(filename.c_str());
  TypeMap t = parseTypeTable(doc);
  std::stringstream ss;
  buildCode(doc, ss);
  std::cout << ss.str() << std::endl;
  return 0;
}

