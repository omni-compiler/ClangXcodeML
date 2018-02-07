#ifndef XCODEMLNAME_H
#define XCODEMLNAME_H

namespace XcodeMl {

class Environment;
class Nns;

/*!
 * \brief Represents the kinds of C++ _unqualified-id_ ([expr.prim.id.unqual]).
 *
 * NOTE: non-exhaustive (example: _literal-operator-id_)
 */
enum class UnqualIdKind {
  /*! _Identifier_ (an sequence of alphabets, digits or underscores) */
  Ident,
  /*! C++ _operator-function-id_ (`operator+`, `operator()`, ...) */
  OpFuncId,
  /*! C++ _conversion-function-id_ */
  ConvFuncId,
  /*! Constructor name */
  Ctor,
  /*! Destructor name */
  Dtor,
};

/*!
 * \brief Represents C++ _unqualified-id_ ([expr.prim.id.unqual]),
 *
 * This class uses LLVM-style RTTI.
 */
class UnqualId {
public:
  UnqualId(UnqualIdKind);
  virtual ~UnqualId() = 0;
  virtual UnqualId *clone() const = 0;
  /*!
   * \brief Returns the source-code representation,
   * like `abc`, `~A`, or `operator+`.
   */
  virtual CodeFragment toString(const Environment &) const = 0;
  UnqualIdKind getKind() const;

protected:
  UnqualId(const UnqualId &) = default;

private:
  UnqualIdKind kind;
};

/**
 * \brief Represents C++ name.
 *
 * A name is a pair of _nested-name-specifier_ and _unqualified-id_.
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

/*! \brief Represents C++ _identifier_. */
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

/*! \brief Represents C++ _operator-function-id_. */
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

/*! \brief Represents C++ _conversion-function-id_. */
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

/*! \brief Represents constructor name. */
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

/*! \brief Represents destructor name. */
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
