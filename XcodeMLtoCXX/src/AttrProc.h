#ifndef ATTRPROC_H
#define ATTRPROC_H

template<typename ReturnT, typename... T>
class AttrProc {
public:
  using Procedure = std::function<ReturnT(xmlNodePtr, T...)>;
  AttrProc() = delete;

  AttrProc(
      const std::string& a,
      std::function<ReturnT(const std::vector<ReturnT>&)> f,
      std::function<ReturnT()> e,
      std::initializer_list<std::tuple<std::string, Procedure>> m):
    attr(a),
    fold(f),
    empty(e),
    map(m)
  {}

  AttrProc(
      const std::string& a,
      std::function<ReturnT(const std::vector<ReturnT>&)> f,
      std::function<ReturnT()> e,
      std::map<std::string, Procedure>&& m):
    attr(a),
    fold(f),
    empty(e),
    map(m)
  {}

  ReturnT walk(xmlNodePtr node, T... args) const {
    std::string getProp(xmlNodePtr, const std::string&);
    assert(node && node->type == XML_ELEMENT_NODE);
    std::string prop(getProp(node, attr));
    auto iter = map.find(prop);
    if (iter != map.end()) {
      return (iter->second)(node, args...);
    }
    return empty();
  }

  std::vector<ReturnT> walkAll(xmlNodePtr node, T... args) const {
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

private:
  std::string attr;
  std::function<ReturnT(const std::vector<ReturnT>&)> fold;
  std::function<ReturnT()> empty;
  std::map<std::string, Procedure> map;
};


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
