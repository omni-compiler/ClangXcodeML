#ifndef ATTRPROC_H
#define ATTRPROC_H

template<typename... T>
class AttrProc {
public:
  using Procedure = std::function<void(xmlNodePtr, T...)>;
  AttrProc() = delete;
  AttrProc(
      const std::string&,
      std::initializer_list<std::tuple<std::string, Procedure>>);
  AttrProc(const std::string&, std::map<std::string, Procedure>&&);
  void walk(xmlNodePtr, T...) const;
  void walkAll(xmlNodePtr, T...) const;
private:
  std::string attr;
  std::map<std::string, Procedure> map;
};

template<typename... T>
AttrProc<T...>::AttrProc(
    const std::string& attr_,
    std::initializer_list<
      std::tuple<std::string, AttrProc::Procedure>> map_):
  attr(attr_),
  map(map_)
{}

template<typename... T>
AttrProc<T...>::AttrProc(
    const std::string& attr_,
    std::map<std::string, Procedure> && map_):
  attr(attr_),
  map(map_)
{}

template<typename... T>
void AttrProc<T...>::walk(xmlNodePtr node, T... args) const {
  assert(node && node->type == XML_ELEMENT_NODE);
  XMLString prop(xmlGetProp(node, BAD_CAST attr.c_str()));
  auto iter = map.find(prop);
  if (iter != map.end()) {
    (iter->second)(node, args...);
  }
}

template<typename... T>
void AttrProc<T...>::walkAll(xmlNodePtr node, T... args) const {
  if (!node) {
    return;
  }
  for (xmlNodePtr cur = xmlFirstElementChild(node);
       cur;
       cur = xmlNextElementSibling(cur))
  {
    walk(cur, args...);
  }
}

#endif /* !ATTRPROC_H */
