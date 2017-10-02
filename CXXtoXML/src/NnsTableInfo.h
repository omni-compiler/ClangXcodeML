#ifndef NNSTABLEINFO_H
#define NNSTABLEINFO_H

class TypeTableInfo;

class NnsTableInfo {
public:
  NnsTableInfo() = delete;
  NnsTableInfo(const NnsTableInfo &) = delete;
  NnsTableInfo(NnsTableInfo &&) = delete;
  NnsTableInfo &operator=(const NnsTableInfo &&) = delete;
  NnsTableInfo &operator=(NnsTableInfo &&) = delete;

  explicit NnsTableInfo(clang::MangleContext *, TypeTableInfo *);
  std::string getNnsName(const clang::NestedNameSpecifier *);
  void popNnsTableStack();
  void pushNnsTableStack(xmlNodePtr);

private:
  xmlNodePtr getNnsNode(const clang::NestedNameSpecifier *) const;
  void pushNns(const clang::NestedNameSpecifier *);
  void registerNestedNameSpec(const clang::NestedNameSpecifier *);

private:
  int seqForOther;
  clang::MangleContext *mangleContext;
  TypeTableInfo *typetableinfo;
  std::map<const clang::NestedNameSpecifier *, std::string> mapForOtherNns;
  std::map<const clang::NestedNameSpecifier *, xmlNodePtr>
      mapFromNestedNameSpecToXmlNodePtr;
  std::stack<std::tuple<xmlNodePtr,
      std::vector<const clang::NestedNameSpecifier *>>> nnsTableStack;
};

#endif
