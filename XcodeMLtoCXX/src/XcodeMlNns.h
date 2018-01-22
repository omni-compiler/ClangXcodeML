#ifndef XCODEMLNNS_H
#define XCODEMLNNS_H

namespace XcodeMl {

class Nns;
using NnsRef = std::shared_ptr<Nns>;

using CodeFragment = CXXCodeGen::StringTreeRef;

using DataTypeIdent = std::string;

class Environment;

using NnsIdent = std::string;

using NnsMap = std::map<NnsIdent, NnsRef>;

/*!
 * \brief Represents the kinds of XcodeML NNS.
 *
 * NOTE: non-exhaustive (example: namespaceNNS)
 */
enum class NnsKind {
  /*! global namespace */
  Global,
  /*! classNNS */
  Class,
};

/*!
 * \brief Represents XcodeML NNS.
 *
 * This class uses LLVM-style RTTI.
 */
class Nns {
public:
  Nns(NnsKind, const NnsRef &, const NnsIdent &);
  Nns(NnsKind, const NnsIdent &, const NnsIdent &);
  virtual ~Nns() = 0;
  virtual Nns *clone() const = 0;
  NnsKind getKind() const;
  CodeFragment makeDeclaration(const Environment &, const NnsMap &) const;

protected:
  Nns(const Nns &) = default;
  /*!
   * \brief Returns the source-code representation of this XcodeML NNS,
   * like `::`, `::A::B::`, or `::std::vector<int>::`.
   */
  virtual CodeFragment makeNestedNameSpec(
      const Environment &, const NnsMap &) const = 0;
  /*! \brief Returns the prefix of this XcodeML NNS. */
  virtual llvm::Optional<NnsIdent> getParent() const;

private:
  llvm::Optional<NnsIdent> parent;
  NnsKind kind;
  NnsIdent ident;
};

/*!
 * \brief Represents XcodeML NNS corresponding to the global namespace.
 */
class GlobalNns : public Nns {
public:
  GlobalNns();
  ~GlobalNns() override = default;
  Nns *clone() const override;
  static bool classof(const Nns *);

protected:
  GlobalNns(const GlobalNns &) = default;
  CodeFragment makeNestedNameSpec(
      const Environment &, const NnsMap &) const override;
  llvm::Optional<NnsIdent> getParent() const override;
};

/*!
 * \brief Represents XcodeML classNNS, an NNS corresponding to a C++ class.
 *
 * Example: An XcodeML constructor name has a classNNS corresponding to the
 * class.
 */
class ClassNns : public Nns {
public:
  ClassNns(const NnsIdent &, const NnsRef &, const DataTypeIdent &);
  ~ClassNns() override = default;
  Nns *clone() const override;
  static bool classof(const Nns *);

protected:
  ClassNns(const ClassNns &) = default;
  virtual CodeFragment makeNestedNameSpec(
      const Environment &, const NnsMap &) const override;

private:
  /*! XcodeML data type identifier of the class */
  DataTypeIdent dtident;
};

/*! \brief Make and return the XcodeML globalNNS. */
NnsRef makeGlobalNns();
NnsRef makeClassNns(const NnsIdent &, const NnsRef &, const DataTypeIdent &);
NnsRef makeClassNns(const NnsIdent &, const DataTypeIdent &);
}

#endif /* !XCODEMLNNS_H */
