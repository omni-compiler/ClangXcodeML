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
#include "llvm/ADT/Optional.h"
#include "StringTree.h"
#include "XMLString.h"
#include "XcodeMlNns.h"
#include "XcodeMlType.h"
#include "XcodeMlTypeTable.h"
#include "XMLWalker.h"
#include "TypeAnalyzer.h"
#include "SourceInfo.h"
#include "CodeBuilder.h"

int
main(int argc, char **argv) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " <filename>" << std::endl;
    return 0;
  }
  std::string filename(argv[1]);
  xmlDocPtr doc = xmlReadFile(filename.c_str(), NULL, XML_PARSE_BIG_LINES);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  xmlXPathContextPtr ctxt = xmlXPathNewContext(doc);
  std::stringstream ss;
  try{
    buildCode(root, ctxt, ss);
  }catch(std::exception &e){
    std::cerr <<e.what()<<std::endl;
    exit(-1);
  }catch(...){
    std::cerr << "Unknown Error"<<std::endl;
    exit(-1);
  }
  std::cout << ss.str() << std::endl;
  xmlXPathFreeContext(ctxt);
  xmlFreeDoc(doc);
  return 0;
}
