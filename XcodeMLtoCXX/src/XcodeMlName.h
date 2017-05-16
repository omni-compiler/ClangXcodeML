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

/**
 * corresponds to the name elements in XcodeML, such as.
 */
class Name {
private:
  UnqualId id;
  Nns nns;
};

class UIDIdent : public UnqualId {
public:
  UIDIdent(const std::string&);
  ~UIDIdent() override = default;
  UnqualId* clone() const override;
  static bool classof(const UnqualId*);
protected:
  Ident(const Ident&) = default;
private:
  std::string ident;
};

class OpFuncId : public UnqualId {
public:
  OpFuncId(const std::string&);
  ~OpFuncId() override = default;
  UnqualId* clone() const override;
  static bool classof(const UnqualId*);
protected:
  OpFuncId(const OpFuncId&) = default;
private:
  std::string opName;
};

class ConvFuncId : public UnqualId {
public:
  ConvFuncId(const DataTypeIdent&);
  ~ConvFuncId() override = default;
  UnqualId* clone() const override;
  static bool classof(const UnqualId*);
protected:
  ConvFuncId(const ConvFuncId&) = default;
private:
  DataTypeIdent dtident;
};

} // namespace XcodeMl

#endif /*! XCODEMLNAME_H */
