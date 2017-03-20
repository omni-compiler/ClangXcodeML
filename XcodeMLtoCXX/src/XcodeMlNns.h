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

enum class NnsKind {
  /*! global namespace */
  Global,
  /*! classNNS */
  Class,
};

class Nns {
public:
  Nns(NnsKind, const NnsRef&, const NnsIdent&);
  virtual ~Nns() = 0;
  virtual Nns* clone() const = 0;
  NnsKind getKind() const;
  CodeFragment makeDeclaration(const Environment&) const;
protected:
  Nns(const Nns&) = default;
  virtual CodeFragment makeNestedNameSpec(const Environment&) const = 0;
  virtual NnsRef getParent() const;
private:
  llvm::Optional<NnsIdent> parent;
  NnsKind kind;
  NnsIdent ident;
};

class GlobalNns : public Nns {
public:
  GlobalNns();
  ~GlobalNns() override = default;
  Nns* clone() const override;
  static bool classof(const Nns *);
protected:
  GlobalNns(const GlobalNns&) = default;
  CodeFragment makeNestedNameSpec(const Environment&) const override;
  NnsRef getParent() const override;
};

class ClassNns : public Nns {
public:
  ClassNns(const NnsIdent&, const NnsRef&, const DataTypeIdent&);
  ~ClassNns() override = default;
  Nns* clone() const override;
  static bool classof(const Nns *);
protected:
  ClassNns(const ClassNns&) = default;
  virtual CodeFragment makeNestedNameSpec(const Environment&)
    const override;
private:
  DataTypeIdent dtident;
};

}

#endif /* !XCODEMLNNS_H */
