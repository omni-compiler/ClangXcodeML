#ifndef CLANGASTWALKER_H
#define CLANGASTWALKER_H

/*!
 * \brief A class that combines procedures into a single one
 * that traverses XML node and visits each element.
 * \tparam ...T parameter type required by procedures in ClangASTWalker.
 *
 * It has a mapping from XML element names to procedures
 * (std::function<void(xmlNodePtr, const ClangASTWalker&, T...)>). Once
 * ClangASTWalker<T...>::walkAll() runs, it performs pre-order traversal of
 * given XML elements and their descendants until it finds an element
 * whose name is registered with the map. Finally it executes
 * a corresponding procedure.
 */
template<typename... T>
class ClangASTWalker {
public:
  using Key = std::tuple<std::string, std::string>;
  /*!
   * \brief Procedure to be registered with ClangASTWalker.
   */
  using Procedure = std::function<void(const ClangASTWalker&, xmlNodePtr, T...)>;
  ClangASTWalker() = default;
  ClangASTWalker(std::initializer_list<std::tuple<Key, Procedure>>);
  ClangASTWalker(std::map<Key, Procedure>&&);
  void walkAll(xmlNodePtr, T...) const;
  void walkChildren(xmlNodePtr, T...) const;
  void walk(xmlNodePtr, T...) const;
  bool registerProc(Key, Procedure);
private:
  std::map<Key, Procedure> map;
};

template<typename... T>
ClangASTWalker<T...>::ClangASTWalker(std::map<Key,Procedure>&& initMap):
  map(initMap)
{}

template<typename... T>
ClangASTWalker<T...>::ClangASTWalker(std::initializer_list<std::tuple<Key, typename ClangASTWalker<T...>::Procedure>> pairs):
  map()
{
  for (auto p : pairs) {
    registerProc(std::get<0>(p), std::get<1>(p));
  }
}

/*!
 * \brief Traverse a given XML element and subsequent elements.
 * \param node XML element to traverse first
 * \param args... Arguments to be passed to registered procedures.
 */
template<typename... T>
void ClangASTWalker<T...>::walkAll(xmlNodePtr node, T... args) const {
  if (!node) {
    return;
  }
  for (xmlNodePtr cur =
        node->type == XML_ELEMENT_NODE ?
          node : xmlNextElementSibling(node) ;
       cur;
       cur = xmlNextElementSibling(cur))
  {
    walk(cur, args...);
  }
}

template<typename... T>
void ClangASTWalker<T...>::walkChildren(xmlNodePtr node, T... args) const {
  if (node) {
    walkAll(node->children, args...);
  }
}

/*!
 * \brief Traverse an XML element.
 * \param node XML element to traverse
 * \param args... Arguments to be passed to registered procedures.
 * \pre \c node is not null.
 * \pre \c node is an XML element node.
 */
template<typename... T>
void ClangASTWalker<T...>::walk(xmlNodePtr node, T... args) const {
  assert(node && node->type == XML_ELEMENT_NODE);
  bool traverseChildren = true;
  XMLString elemName = node->name;
  auto iter = map.find(elemName);
  if (iter != map.end()) {
    (iter->second)(*this, node, args...);
    traverseChildren = false;
  }
  if (traverseChildren) {
    walkAll(node->children, args...);
  }
}

/*!
 * \brief Register a procedure. If \c key already exists,
 * do nothing.
 * \param key The name of XML element which \c proc should process.
 * \param value Procedure to run.
 * \return false if \c key already exists.
 */
template<typename... T>
bool ClangASTWalker<T...>::registerProc(Key key, Procedure value) {
  auto iter = map.find(key);
  if (iter != map.end()) {
    return false;
  }
  map[key] = value;
  return true;
}

#endif /* !CLANGASTWALKER_H */
