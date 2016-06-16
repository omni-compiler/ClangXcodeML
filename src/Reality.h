class Reality;
using NodeProcessor = std::function<void(xmlNodePtr, xmlXPathContextPtr, std::stringstream&, const Reality&)>;

class Reality {
public:
  void call(xmlNodePtr, xmlXPathContextPtr, std::stringstream&) const;
  bool registerNP(std::string, NodeProcessor);
private:
  std::map<std::string, NodeProcessor> map;
};

void Reality::call(xmlNodePtr node, xmlXPathContextPtr ctxt, std::stringstream& ss) const {
  for (xmlNodePtr cur = node; cur; cur = cur->next) {
    bool traverseChildren = true;
    if (cur->type == XML_ELEMENT_NODE) {
      XMLString elemName = cur->name;
      std::cout << "/* "<< static_cast<std::string>( elemName ) << " */\n";
      auto iter = map.find(elemName);
      if (iter != map.end()) {
        (iter->second)(cur, ctxt, ss, *this);
        traverseChildren = false;
      }
    }
    if (traverseChildren) {
      call(cur->children, ctxt, ss);
    }
  }
}

bool Reality::registerNP(std::string key, NodeProcessor proc) {
  auto iter = map.find(key);
  if (iter != map.end()) {
    return false;
  }
  map[key] = proc;
  return true;
}


