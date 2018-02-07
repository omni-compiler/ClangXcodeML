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
  /*! otherNNS */
  Other,
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
  Nns(NnsKind kind, const NnsIdent &ident);
  Nns(NnsKind kind, const NnsRef &parent, const NnsIdent &ident);
  Nns(NnsKind kind, const NnsIdent &parent, const NnsIdent &ident);
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

class OtherNns : public Nns {
public:
  OtherNns(const NnsIdent &);
  ~OtherNns() override = default;
  Nns *clone() const override;
  static bool classof(const Nns *);

protected:
  OtherNns(const OtherNns &) = default;
  virtual CodeFragment makeNestedNameSpec(
      const Environment &, const NnsMap &) const override;
};

/*! \brief Make and return the XcodeML globalNNS. */
NnsRef makeGlobalNns();

/*!
 * \brief Make and return an `XcodeMl::ClassNns` object.
 *
 * \param nident the NNS identifier
 * \param prefix the prefix (or parent) NNS
 * \param classType the data type identifier of the corresponding class
 */
NnsRef makeClassNns(const NnsIdent &nident,
    const NnsRef &prefix,
    const DataTypeIdent &classType);

/*!
 * \brief Make and return an `XcodeMl::ClassNns` object that does not have
 * prefix NNS.
 *
 * \param nident the NNS identifier
 * \param classType the data type identifier of the corresponding class
 */
NnsRef makeClassNns(const NnsIdent &nident, const DataTypeIdent &classType);

NnsRef makeOtherNns(const NnsIdent &nident);

} // namespace XcodeMl

#endif /* !XCODEMLNNS_H */
