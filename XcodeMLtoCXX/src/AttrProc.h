#ifndef ATTRPROC_H
#define ATTRPROC_H

template<typename ReturnT, typename... T>
class AttrProc {
public:
  using Procedure = std::function<ReturnT(xmlNodePtr, T...)>;
  AttrProc() = delete;
  AttrProc(
      const std::string&,
      std::function<ReturnT(const std::vector<ReturnT>&)>,
      std::function<ReturnT()>,
      std::initializer_list<std::tuple<std::string, Procedure>>);
  AttrProc(
      const std::string&,
      std::function<ReturnT(const std::vector<ReturnT>&)>,
      std::function<ReturnT()>,
      std::map<std::string, Procedure>&&);
  ReturnT walk(xmlNodePtr, T...) const;
  std::vector<ReturnT> walkAll(xmlNodePtr, T...) const;
private:
  std::string attr;
  std::function<ReturnT(const std::vector<ReturnT>&)> fold;
  std::function<ReturnT()> empty;
  std::map<std::string, Procedure> map;
};

template<typename ReturnT, typename... T>
AttrProc<ReturnT, T...>::AttrProc(
    const std::string& attr_,
    std::function<ReturnT(const std::vector<ReturnT>&)> fold_,
    std::function<ReturnT()> empty_,
    std::initializer_list<
      std::tuple<std::string, AttrProc::Procedure>> map_):
  attr(attr_),
  fold(fold_),
  empty(empty_),
  map(map_)
{}

template<typename ReturnT, typename... T>
AttrProc<ReturnT, T...>::AttrProc(
    const std::string& attr_,
    std::function<ReturnT(const std::vector<ReturnT>&)> fold_,
    std::function<ReturnT()> empty_,
    std::map<std::string, Procedure> && map_):
  attr(attr_),
  fold(fold_),
  empty(empty_),
  map(map_)
{}

template<typename ReturnT, typename... T>
ReturnT AttrProc<ReturnT, T...>::walk(xmlNodePtr node, T... args) const {
  assert(node && node->type == XML_ELEMENT_NODE);
  XMLString prop(xmlGetProp(node, BAD_CAST attr.c_str()));
  auto iter = map.find(prop);
  if (iter != map.end()) {
    return (iter->second)(node, args...);
  }
  return empty();
}

template<typename ReturnT, typename... T>
std::vector<ReturnT> AttrProc<ReturnT, T...>::walkAll(xmlNodePtr node, T... args) const {
  if (!node) {
    return {};
  }
  std::vector<ReturnT> ret;
  for (xmlNodePtr cur = xmlFirstElementChild(node);
       cur;
       cur = xmlNextElementSibling(cur))
  {
    ret.push_back(walk(cur, args...));
  }
  return ret;
}

template<typename... T>
class AttrProc<void, T...> {
public:
  using Procedure = std::function<void(xmlNodePtr, T...)>;
  AttrProc() = delete;
  AttrProc(
      const std::string& a,
      std::initializer_list<std::tuple<std::string, Procedure>> m):
    attr(a),
    map(m)
  {}

  AttrProc(
      const std::string& a,
      std::map<std::string, Procedure>&& m):
    attr(a),
    map(m)
  {}

  void walk(xmlNodePtr node, T... args) const {
    assert(node && node->type == XML_ELEMENT_NODE);
    XMLString prop(xmlGetProp(node, BAD_CAST attr.c_str()));
    auto iter = map.find(prop);
    if (iter != map.end()) {
      (iter->second)(node, args...);
    }
  }

  void walkAll(xmlNodePtr node, T... args) const {
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

private:
  std::string attr;
  std::map<std::string, Procedure> map;
};

#endif /* !ATTRPROC_H */
