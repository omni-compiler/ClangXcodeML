class Reality;
using NodeProcessor = std::function<void(xmlNodePtr, xmlXPathContextPtr, std::stringstream&, const Reality&)>;

class Reality {
public:
  void call(xmlNodePtr, xmlXPathContextPtr, std::stringstream&) const;
  void callOnce(xmlNodePtr, xmlXPathContextPtr, std::stringstream&) const;
  bool registerNP(std::string, NodeProcessor);
private:
  std::map<std::string, NodeProcessor> map;
};

void Reality::call(xmlNodePtr node, xmlXPathContextPtr ctxt, std::stringstream& ss) const {
  for (xmlNodePtr cur = node; cur; cur = cur->next) {
    callOnce(cur, ctxt, ss);
  }
}

void Reality::callOnce(xmlNodePtr node, xmlXPathContextPtr ctxt, std::stringstream& ss) const {
  bool traverseChildren = true;
  if (node->type == XML_ELEMENT_NODE) {
    XMLString elemName = node->name;
    auto iter = map.find(elemName);
    if (iter != map.end()) {
      (iter->second)(node, ctxt, ss, *this);
      traverseChildren = false;
    }
  }
  if (traverseChildren) {
    call(node->children, ctxt, ss);
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


