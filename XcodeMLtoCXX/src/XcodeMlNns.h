#ifndef XCODEMLNNS_H
#define XCODEMLNNS_H

namespace XcodeMl {

class Nns;
using NnsRef = std::shared_ptr<Nns>;

using CodeFragment = CXXCodeGen::StringTreeRef;

using DataTypeIdent = std::string;

class Environment;

using NnsIdent = std::string;

enum class NnsKind {
  /*! classNNS */
  Class,
};

class Nns {
public:
  Nns(NnsKind, const NnsRef&, const NnsIdent&);
  virtual ~Nns() = 0;
  virtual Nns* clone() const = 0;
  NnsKind getKind() const;
  virtual CodeFragment makeDeclaration(const Environment&) const = 0;
protected:
  Nns(const Nns&) = default;
private:
  NnsRef parent;
  NnsKind kind;
  NnsIdent ident;
};

class ClassNns : public Nns {
public:
  ClassNns(const NnsIdent&, const NnsRef&, const DataTypeIdent&);
  ~ClassNns() override = default;
  Nns* clone() const override;
  virtual CodeFragment makeDeclaration(const Environment&) const override;
  static bool classof(const Nns *);
protected:
  ClassNns(const ClassNns&) = default;
private:
  DataTypeIdent dtident;
};

}

#endif /* !XCODEMLNNS_H */
