#ifndef NNSTABLEINFO_H
#define NNSTABLEINFO_H

class NnsTableInfo {
public:
  NnsTableInfo() = delete;
  NnsTableInfo(const NnsTableInfo&) = delete;
  NnsTableInfo(NnsTableInfo&&) = delete;
  NnsTableInfo& operator =(const NnsTableInfo&&) = delete;
  NnsTableInfo& operator =(NnsTableInfo&&) = delete;

  explicit NnsTableInfo(clang::MangleContext*);
  std::string getNnsName(clang::NestedNameSpecifier*);
private:
  void registerNestedNameSpec(clang::NestedNameSpecifier*);

private:
  int seqForOther;
  clang::MangleContext* MC;
  std::map<clang::NestedNameSpecifier*, std::string> mapForOtherNns;
};

#endif
