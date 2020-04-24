#ifndef XMLWALKER_H
#define XMLWALKER_H

#include <libxml/debugXML.h>
#include <iostream>

/*!
 * \brief A class that combines procedures into a single one
 * that traverses XML node and visits each element.
 * \tparam ...T parameter type required by procedures in XMLWalker.
 *
 * It has a mapping from XML element names to procedures
 * (std::function<void(xmlNodePtr, const XMLWalker&, T...)>). Once
 * XMLWalker<T...>::walkAll() runs, it performs pre-order traversal of
 * given XML elements and their descendants until it finds an element
 * whose name is registered with the map. Finally it executes
 * a corresponding procedure.
 */
template <typename ReturnT, typename... T>
class XMLWalker {
public:
  /*!
   * \brief Procedure to be registered with XMLWalker.
   */
  using Procedure =
      std::function<ReturnT(const XMLWalker &, xmlNodePtr, T...)>;

  XMLWalker(const std::string &n,
      const std::function<ReturnT(const std::vector<ReturnT> &)> f,
      std::initializer_list<std::tuple<std::string, Procedure>> pairs)
      : name(n), fold(f), map() {
    for (auto p : pairs) {
      registerProc(std::get<0>(p), std::get<1>(p));
    }
  }

  XMLWalker(const std::string &n,
      const std::function<ReturnT(const std::vector<ReturnT> &)> f,
      std::map<std::string, Procedure> &&initMap)
      : name(n), fold(f), map(initMap) {
  }

  const Procedure &operator[](const std::string &key) const {
    const auto iter = map.find(key);
    if (iter == map.end()) {
      std::cerr << "In " << name << ":" << std::endl
                << "Nonexistent procedure called: '" + key + "'" << std::endl;
      std::abort();
    }
    return iter->second;
  }

  /*!
   * \brief Traverse a given XML element and subsequent elements.
   * \param node XML element to traverse first
   * \param args... Arguments to be passed to registered procedures.
   */
  std::vector<ReturnT>
  walkAll(xmlNodePtr node, T... args) const {
    std::vector<ReturnT> values;
    if (!node) {
      return values;
    }
    for (xmlNodePtr cur = node->type == XML_ELEMENT_NODE
             ? node
             : xmlNextElementSibling(node);
         cur;
         cur = xmlNextElementSibling(cur)) {
      values.push_back(walk(cur, args...));
    }
    return values;
  }

  std::vector<ReturnT>
  walkChildren(xmlNodePtr node, T... args) const {
    if (node) {
      return walkAll(node->children, args...);
    } else {
      return {};
    }
  }

  /*!
   * \brief Traverse an XML element.
   * \param node XML element to traverse
   * \param args... Arguments to be passed to registered procedures.
   * \pre \c node is not null.
   * \pre \c node is an XML element node.
   */
  ReturnT
  walk(xmlNodePtr node, T... args) const
  {
    if(!node){
      throw std::runtime_error("null node passed");
    }else if(node->type != XML_ELEMENT_NODE){
      std::cerr <<"Node Type Invalid"<<node->name<<node->type<<std::endl;
      throw std::runtime_error("Node Type Invalid");
    }
    XMLString elemName = node->name;
    auto iter = map.find(elemName);
    try{
      if (iter != map.end()) {
	return (iter->second)(*this, node, args...);
      } else {
	return fold(walkAll(node->children, args...));
      }
    }catch(std::exception &e){
        std::cerr << "In " << name << ": walk(" << elemName << ")" << std::endl << e.what() << std::endl;
	std::cerr<< node->name <<xmlGetLineNo(node)<<std::endl;
      throw ;
    }
  }

  /*!
   * \brief Register a procedure. If \c key already exists,
   * do nothing.
   * \param key The name of XML element which \c proc should process.
   * \param value Procedure to run.
   * \return false if \c key already exists.
   */
  bool
  registerProc(std::string key, Procedure value) {
    auto iter = map.find(key);
    if (iter != map.end()) {
      return false;
    }
    map[key] = value;
    return true;
  }

private:
  std::string name;
  std::function<ReturnT(const std::vector<ReturnT> &)> fold;
  std::map<std::string, Procedure> map;
};

template <typename... T>
class XMLWalker<void, T...> {
public:
  using Procedure = std::function<void(const XMLWalker &, xmlNodePtr, T...)>;

  XMLWalker(const std::string &n,
      std::initializer_list<std::tuple<std::string, Procedure>> pairs)
      : name(n), map() {
    for (auto p : pairs) {
      registerProc(std::get<0>(p), std::get<1>(p));
    }
  }

  XMLWalker(const std::string &n, std::map<std::string, Procedure> &&initMap)
      : name(n), map(initMap) {
  }

  const Procedure &operator[](const std::string &key) const {
    const auto iter = map.find(key);
    if (iter == map.end()) {
      std::cerr << "In " << name << ":" << std::endl
                << "Nonexistent procedure called: '" + key + "'" << std::endl;
      std::abort();
    }
    return iter->second;
  }

  void
  walkAll(xmlNodePtr node, T... args) const {
    if (!node) {
      return;
    }
    for (xmlNodePtr cur = node->type == XML_ELEMENT_NODE
             ? node
             : xmlNextElementSibling(node);
         cur;
         cur = xmlNextElementSibling(cur)) {
      walk(cur, args...);
    }
  }

  void
  walkChildren(xmlNodePtr node, T... args) const {
    if (node) {
      walkAll(node->children, args...);
    }
  }

  void
  walk(xmlNodePtr node, T... args) const {
    if(!node){
      throw std::runtime_error("null node passed");
    }else if(node->type != XML_ELEMENT_NODE){
      std::cerr <<"Node Type Invalid"<<node->name<<node->type<<std::endl;
      throw std::runtime_error("Node Type Invalid");
    }
    XMLString elemName = node->name;
    auto iter = map.find(elemName);
    if (iter != map.end()) {
      try {
        (iter->second)(*this, node, args...);
      } catch (const std::exception &e) {
        std::cerr << "In " << name << ": walk(" << elemName << ")" << std::endl << e.what() << std::endl;
        xmlDebugDumpNode(stderr, node, 0);
        abort();
      }
    } else {
      walkAll(node->children, args...);
    }
  }

  bool
  registerProc(std::string key, Procedure value) {
    auto iter = map.find(key);
    if (iter != map.end()) {
      return false;
    }
    map[key] = value;
    return true;
  }

private:
  std::string name;
  std::map<std::string, Procedure> map;
};

#endif /* !XMLWALKER_H */
