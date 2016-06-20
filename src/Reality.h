class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  std::map<std::string, std::string> typeTable;
};

class Reality;
using NodeProcessor = std::function<void(xmlNodePtr, SourceInfo, std::stringstream&, const Reality&)>;

class Reality {
public:
  void call(xmlNodePtr, SourceInfo, std::stringstream&) const;
  void callOnce(xmlNodePtr, SourceInfo, std::stringstream&) const;
  bool registerNP(std::string, NodeProcessor);
private:
  std::map<std::string, NodeProcessor> map;
};

void Reality::call(xmlNodePtr node, SourceInfo src, std::stringstream& ss) const {
  for (xmlNodePtr cur = node; cur; cur = cur->next) {
    callOnce(cur, src, ss);
  }
}

void Reality::callOnce(xmlNodePtr node, SourceInfo src, std::stringstream& ss) const {
  bool traverseChildren = true;
  if (node->type == XML_ELEMENT_NODE) {
    XMLString elemName = node->name;
    auto iter = map.find(elemName);
    if (iter != map.end()) {
      (iter->second)(node, src, ss, *this);
      traverseChildren = false;
    }
  }
  if (traverseChildren) {
    call(node->children, src, ss);
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

