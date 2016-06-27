#ifndef REALITY_H
#define REALITY_H

template<typename... T>
class Reality {
public:
  using Procedure = std::function<void(xmlNodePtr, const Reality&, T...)>;
  void call(xmlNodePtr, T...) const;
  void callOnce(xmlNodePtr, T...) const;
  bool registerNP(std::string, Procedure);
  static Procedure merge(Procedure, Procedure);
private:
  std::map<std::string, Procedure> map;
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
bool Reality<T...>::registerNP(std::string key, Procedure proc) {
  auto iter = map.find(key);
  if (iter != map.end()) {
    return false;
  }
  map[key] = proc;
  return true;
}

template<typename... T>
typename Reality<T...>::Procedure Reality<T...>::merge(
    typename Reality<T...>::Procedure lhs,
    typename Reality<T...>::Procedure rhs) {
  return [lhs, rhs](xmlNodePtr node, const Reality<T...>& r, T... args) {
    lhs(node, r, args ...);
    rhs(node, r, args ...);
  };
}

#endif /* !REALITY_H */
