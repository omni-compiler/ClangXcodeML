#ifndef XCODEMLNAME_H
#define XCODEMLNAME_H

namespace XcodeMl {

class Environment;
class Nns;

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
  virtual UnqualId *clone() const = 0;
  virtual CodeFragment toString(const Environment &) const = 0;
  UnqualIdKind getKind() const;

protected:
  UnqualId(const UnqualId &) = default;

private:
  UnqualIdKind kind;
};

/**
 * corresponds to the name elements in XcodeML, such as.
 */
class Name {
public:
  explicit Name(
      const std::shared_ptr<UnqualId> &, const std::shared_ptr<Nns> &);
  CodeFragment toString(const Environment &, const NnsMap &) const;
  std::shared_ptr<UnqualId> getUnqualId() const;

private:
  std::shared_ptr<UnqualId> id;
  std::shared_ptr<Nns> nns;
};

class UIDIdent : public UnqualId {
public:
  UIDIdent(const std::string &);
  ~UIDIdent() override = default;
  UnqualId *clone() const override;
  CodeFragment toString(const Environment &) const override;
  static bool classof(const UnqualId *);

protected:
  UIDIdent(const UIDIdent &) = default;

private:
  std::string ident;
};

class OpFuncId : public UnqualId {
public:
  OpFuncId(const std::string &);
  ~OpFuncId() override = default;
  UnqualId *clone() const override;
  CodeFragment toString(const Environment &) const override;
  static bool classof(const UnqualId *);

protected:
  OpFuncId(const OpFuncId &) = default;

private:
  std::string opSpelling;
};

class ConvFuncId : public UnqualId {
public:
  ConvFuncId(const DataTypeIdent &);
  ~ConvFuncId() override = default;
  UnqualId *clone() const override;
  CodeFragment toString(const Environment &) const override;
  static bool classof(const UnqualId *);

protected:
  ConvFuncId(const ConvFuncId &) = default;

private:
  DataTypeIdent dtident;
};

class CtorName : public UnqualId {
public:
  CtorName(const DataTypeIdent &);
  ~CtorName() override = default;
  UnqualId *clone() const override;
  CodeFragment toString(const Environment &) const override;
  static bool classof(const UnqualId *);

protected:
  CtorName(const CtorName &) = default;

private:
  DataTypeIdent dtident;
};

class DtorName : public UnqualId {
public:
  DtorName(const DataTypeIdent &);
  ~DtorName() override = default;
  UnqualId *clone() const override;
  CodeFragment toString(const Environment &) const override;
  static bool classof(const UnqualId *);

protected:
  DtorName(const DtorName &) = default;

private:
  DataTypeIdent dtident;
};

} // namespace XcodeMl

#endif /*! XCODEMLNAME_H */
