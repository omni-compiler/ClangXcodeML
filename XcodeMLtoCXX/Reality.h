#ifndef REALITY_H
#define REALITY_H

/*!
 * \brief A class that combines procedures into a single one
 * that traverses XML node and visits each element.
 * \tparam ...T parameter type required by procedures in Reality.
 *
 * It has a mapping from XML element names to procedures
 * (std::function<void(xmlNodePtr, const Reality&, T...)>). Once
 * Reality<T...>::call() runs, it performs pre-order traversal of
 * given XML elements and its descendants until it finds an element
 * whose name is registered with the map. Finally it executes
 * a corresponding procedure.
 */
template<typename... T>
class Reality {
public:
  /*!
   * \brief Procedure to be registered with Reality.
   */
  using Procedure = std::function<void(xmlNodePtr, const Reality&, T...)>;
  Reality() = default;
  Reality(std::initializer_list<std::tuple<std::string, Procedure>>);
  void call(xmlNodePtr, T...) const;
  void callOnce(xmlNodePtr, T...) const;
  bool registerNP(std::string, Procedure);
private:
  std::map<std::string, Procedure> map;
};

template<typename... T>
Reality<T...>::Reality(std::initializer_list<std::tuple<std::string, typename Reality<T...>::Procedure>> pairs):
  map()
{
  for (auto p : pairs) {
    registerNP(std::get<0>(p), std::get<1>(p));
  }
}

/*!
 * \brief Traverse a given XML element and subsequent elements.
 * \param node XML element to traverse first
 * \param args... Arguments to be passed to registered procedures.
 */
template<typename... T>
void Reality<T...>::call(xmlNodePtr node, T... args) const {
  for (xmlNodePtr cur = node; cur; cur = cur->next) {
    callOnce(cur, args...);
  }
}

/*!
 * \brief Traverse an XML element.
 * \param node XML element to traverse
 * \param args... Arguments to be passed to registered procedures.
 */
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

/*!
 * \brief Register a procedure. If \c key already exists,
 * do nothing.
 * \param key The name of XML element which \c proc should process.
 * \param value Procedure to run.
 * \return false if \c key already exists.
 */
template<typename... T>
bool Reality<T...>::registerNP(std::string key, Procedure value) {
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

#endif /* !REALITY_H */
