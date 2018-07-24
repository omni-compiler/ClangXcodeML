#ifndef NNSTABLEINFO_H
#define NNSTABLEINFO_H

class TypeTableInfo;

struct NnsTableInfoImpl;

class NnsTableInfo {
public:
  NnsTableInfo() = delete;
  NnsTableInfo(const NnsTableInfo &) = delete;
  NnsTableInfo(NnsTableInfo &&) = delete;
  NnsTableInfo &operator=(const NnsTableInfo &&) = delete;
  NnsTableInfo &operator=(NnsTableInfo &&) = delete;
  ~NnsTableInfo();

  explicit NnsTableInfo(clang::MangleContext *, TypeTableInfo *);
  std::string getNnsName(const clang::DeclContext *);
  void popNnsTableStack();
  void pushNnsTableStack(xmlNodePtr);

private:
  std::unique_ptr<NnsTableInfoImpl> pimpl;
};

#endif
