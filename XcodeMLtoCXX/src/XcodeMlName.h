#ifndef XCODEMLNAME_H
#define XCODEMLNAME_H

namespace XcodeMl {

enum class UnqualIdKind {
  Ident,
  OpFuncId,
  ConvFuncId,
  Ctor,
  Dtor,
};

class UnqualId {
public:
  UnqualId(UnqualIdKind);
  virtual ~UnqualId() = 0;
  virtual UnqualId* clone() const = 0;
  UnqualIdKind getKind() const;
protected:
  UnqualId(const UnqualId&) = default;
private:
  UnqualIdKind kind;
};

class Name {
private:
  UnqualId id;
  Nns nns;
};

class OpFuncId : public UnqualId {
public:
  OpFuncId(const std::string&);
  ~OpFuncId() override = 0;
  UnqualId* clone() const override;
  static bool classof(const UnqualId*);
private:
  std::string opName;
};

class ConvFuncId : public UnqualId {
public:
  ConvFuncId(const std::string&);
};

} // namespace XcodeMl

#endif /*! XCODEMLNAME_H */
