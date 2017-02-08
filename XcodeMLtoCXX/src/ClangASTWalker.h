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
  ClangASTWalker(std::initializer_list<std::tuple<std::string, Procedure>>);
  ClangASTWalker(std::map<std::string, Procedure>&&);
  void walkAll(xmlNodePtr, T...) const;
  void walkChildren(xmlNodePtr, T...) const;
  void walk(xmlNodePtr, T...) const;
  bool registerProc(std::string, Procedure);
private:
  std::map<std::string, Procedure> map;
};

template<typename... T>
ClangASTWalker<T...>::ClangASTWalker(std::map<std::string,Procedure>&& initMap):
  map(initMap)
{}

template<typename... T>
ClangASTWalker<T...>::ClangASTWalker(std::initializer_list<std::tuple<std::string, typename ClangASTWalker<T...>::Procedure>> pairs):
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
bool ClangASTWalker<T...>::registerProc(std::string key, Procedure value) {
  auto iter = map.find(key);
  if (iter != map.end()) {
    return false;
  }
  map[key] = value;
  return true;
}

/*!
 * \brief Merge two procedures into one that executes them in order.
 * \param lhs Procedure that runs first.
 * \param rhs Procedure that runs second.
 */
template<typename... T>
typename std::function<void(T...)> merge(
    typename std::function<void(T...)> lhs,
    typename std::function<void(T...)> rhs) {
  return [lhs, rhs](T... args) {
    lhs(args ...);
    rhs(args ...);
  };
}

#endif /* !CLANGASTWALKER_H */
