class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  std::map<std::string, std::string> typeTable;
};

template<typename... T>
class Reality;

template<typename... T>
using NodeProcessor = std::function<void(xmlNodePtr, const Reality<T...>&, T...)>;

template<typename... T>
class Reality {
public:
  void call(xmlNodePtr, T...) const;
  void callOnce(xmlNodePtr, T...) const;
  bool registerNP(std::string, NodeProcessor<T...>);
private:
  std::map<std::string, NodeProcessor<T...>> map;
};

template<typename... T>
void Reality<T...>::call(xmlNodePtr node, T... args) const {
  for (xmlNodePtr cur = node; cur; cur = cur->next) {
    callOnce(cur, args...);
  }
}

template<typename... T>
void Reality<T...>::callOnce(xmlNodePtr node, T... args) const {
  bool traverseChildren = true;
  if (node->type == XML_ELEMENT_NODE) {
    XMLString elemName = node->name;
    auto iter = map.find(elemName);
    if (iter != map.end()) {
      (iter->second)(node, *this, args...);
      traverseChildren = false;
    }
  }
  if (traverseChildren) {
    call(node->children, args...);
  }
}

template<typename... T>
bool Reality<T...>::registerNP(std::string key, NodeProcessor<T...> proc) {
  auto iter = map.find(key);
  if (iter != map.end()) {
    return false;
  }
  map[key] = proc;
  return true;
}

