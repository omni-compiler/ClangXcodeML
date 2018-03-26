#ifndef XCODEMLNNS_H
#define XCODEMLNNS_H

namespace XcodeMl {

class Nns;
using NnsRef = std::shared_ptr<Nns>;

using CodeFragment = CXXCodeGen::StringTreeRef;

using DataTypeIdent = std::string;

class TypeTable;

using NnsIdent = std::string;

using NnsTable = std::map<NnsIdent, NnsRef>;

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
  /*! namespaceNNS */
  Namespace,
  /*! NamespaceNNS (unnamed) */
  UnnamedNamespace,
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
  CodeFragment makeDeclaration(const TypeTable &, const NnsTable &) const;

protected:
  Nns(const Nns &) = default;
  /*!
   * \brief Returns the source-code representation of this XcodeML NNS,
   * like `::`, `::A::B::`, or `::std::vector<int>::`.
   */
  virtual CodeFragment makeNestedNameSpec(
      const TypeTable &, const NnsTable &) const = 0;
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
      const TypeTable &, const NnsTable &) const override;
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
  ClassNns(const NnsIdent &, const NnsIdent &, const DataTypeIdent &);
  ClassNns(const NnsIdent &, const DataTypeIdent &);
  ~ClassNns() override = default;
  Nns *clone() const override;
  static bool classof(const Nns *);

protected:
  ClassNns(const ClassNns &) = default;
  virtual CodeFragment makeNestedNameSpec(
      const TypeTable &, const NnsTable &) const override;

private:
  /*! XcodeML data type identifier of the class */
  DataTypeIdent dtident;
};

class NamespaceNns : public Nns {
public:
  NamespaceNns(const NnsIdent &nident, const std::string &name);
  NamespaceNns(
      const NnsIdent &nident, const std::string &name, const NnsIdent &parent);
  ~NamespaceNns() override = default;
  Nns *clone() const override;
  static bool classof(const Nns *);

protected:
  NamespaceNns(const NamespaceNns &) = default;
  virtual CodeFragment makeNestedNameSpec(
      const TypeTable &, const NnsTable &) const override;

private:
  std::string name;
};

class UnnamedNamespaceNns : public Nns {
public:
  UnnamedNamespaceNns(const NnsIdent &nident, const NnsIdent &parent)
      : Nns(NnsKind::UnnamedNamespace, parent, nident) {
  }
  ~UnnamedNamespaceNns() override = default;
  Nns *
  clone() const override {
    UnnamedNamespaceNns *copy = new UnnamedNamespaceNns(*this);
    return copy;
  }
  static bool
  classof(const Nns *N) {
    return N->getKind() == NnsKind::UnnamedNamespace;
  }

protected:
  UnnamedNamespaceNns(const UnnamedNamespaceNns &) = default;
  virtual CodeFragment makeNestedNameSpec(
      const TypeTable &, const NnsTable &) const override;
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
      const TypeTable &, const NnsTable &) const override;
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
    const NnsIdent &prefix,
    const DataTypeIdent &classType);

/*!
 * \brief Make and return an `XcodeMl::ClassNns` object that does not have
 * prefix NNS.
 *
 * \param nident the NNS identifier
 * \param classType the data type identifier of the corresponding class
 */
NnsRef makeClassNns(const NnsIdent &nident, const DataTypeIdent &classType);

NnsRef makeNamespaceNns(const NnsIdent &nident, const std::string &name);

NnsRef makeNamespaceNns(
    const NnsIdent &nident, const NnsIdent &parent, const std::string &name);

NnsRef makeUnnamedNamespaceNns(const NnsIdent &nident, const NnsIdent &parent);

NnsRef makeOtherNns(const NnsIdent &nident);

} // namespace XcodeMl

#endif /* !XCODEMLNNS_H */
