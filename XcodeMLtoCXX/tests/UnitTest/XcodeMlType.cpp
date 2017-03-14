#define BOOST_TEST_MODULE XcodeMl::Type
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "StringTree.h"
#include "Symbol.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "XcodeMlType.h"

BOOST_AUTO_TEST_SUITE(xcodeml_type)

static
CXXCodeGen::StringTreeRef wrap(const std::string& s) {
  return CXXCodeGen::makeTokenNode(s);
}

BOOST_AUTO_TEST_CASE(makeXXXType_nullability_test) {
  BOOST_TEST_CHECKPOINT("makeXXXType(...) is not nullable");

  auto intType = XcodeMl::makeReservedType("int", wrap( "int" ));
  BOOST_CHECK(intType);

  BOOST_CHECK(XcodeMl::makePointerType("p1", intType));
  BOOST_CHECK(XcodeMl::makePointerType("p2", "p1"));

  BOOST_CHECK(XcodeMl::makeFunctionType("f1", intType, {}));

  BOOST_CHECK(XcodeMl::makeArrayType("a1", intType, 10));

  BOOST_CHECK(XcodeMl::makeStructType("s1", wrap( "tag" ), {}));
}

BOOST_AUTO_TEST_CASE(makeXXXType_attr_test) {
  BOOST_TEST_CHECKPOINT("Type of `makeXXXType(...)` == XXX");

  using XcodeMl::TypeKind;
  using XcodeMl::typeKind;

  const auto reserved = XcodeMl::makeReservedType("ident1", wrap( "int" ));
  BOOST_CHECK(typeKind(reserved) == TypeKind::Reserved);

  const auto pointer1 = XcodeMl::makePointerType("p1", reserved);
  BOOST_CHECK(typeKind(pointer1) == TypeKind::Pointer);

  const auto pointer2 = XcodeMl::makePointerType("p2", "ident1");
  BOOST_CHECK(typeKind(pointer2) == TypeKind::Pointer);

  const auto array = XcodeMl::makeArrayType("a1", pointer2, 10);
  BOOST_CHECK(typeKind(array) == TypeKind::Array);

  const auto function = XcodeMl::makeFunctionType("f1", reserved, {});
  BOOST_CHECK(typeKind(function) == TypeKind::Function);

  const auto structure = XcodeMl::makeStructType("s1", wrap( "tag" ), {});
  BOOST_CHECK(typeKind(structure) == TypeKind::Struct);


  BOOST_TEST_CHECKPOINT("XcodeMl::Type::dataTypeIdent()");

  BOOST_CHECK_EQUAL(reserved->dataTypeIdent(), "ident1");
  BOOST_CHECK_EQUAL(pointer1->dataTypeIdent(), "p1");
  BOOST_CHECK_EQUAL(pointer2->dataTypeIdent(), "p2");
  BOOST_CHECK_EQUAL(array->dataTypeIdent(), "a1");
  BOOST_CHECK_EQUAL(function->dataTypeIdent(), "f1");
  BOOST_CHECK_EQUAL(structure->dataTypeIdent(), "s1");
}

BOOST_AUTO_TEST_CASE(cv_qualification_test) {
  BOOST_TEST_CHECKPOINT("makeXXXType(...) returns cv-unqualified type");
  auto rsv1 = XcodeMl::makeReservedType("rsv1", wrap("int"));
  std::vector<XcodeMl::TypeRef> types = {
    rsv1,
    XcodeMl::makeReservedType("rsv2", wrap("rsv1")),
    XcodeMl::makePointerType("ptr1", "rsv2"),
    XcodeMl::makePointerType("ptr1", rsv1),
    XcodeMl::makeFunctionType("fun1", rsv1, {}),
    XcodeMl::makeArrayType("arr1", rsv1, 10),
    XcodeMl::makeStructType("str1", wrap("tag"), {})
  };
  for (auto type : types) {
    BOOST_TEST_MESSAGE("Checking cv-qualification of " + type->dataTypeIdent());
    BOOST_CHECK( !(type->isConst()) );
    BOOST_CHECK( !(type->isVolatile()) );
  }

  BOOST_TEST_CHECKPOINT("Type::setConst() changes constness");
  for (auto type : types) {
    BOOST_TEST_MESSAGE("Checking setConst() of " + type->dataTypeIdent());
    type->setConst(true);
    BOOST_CHECK(type->isConst());
    type->setConst(false);
    BOOST_CHECK( !(type->isConst()) );
  }

  BOOST_TEST_CHECKPOINT("Type::setVolatile() changes volatility");
  for (auto type : types) {
    BOOST_TEST_MESSAGE("Checking setVolatile() of " + type->dataTypeIdent());
    type->setVolatile(true);
    BOOST_CHECK(type->isVolatile());
    type->setVolatile(false);
    BOOST_CHECK( !(type->isVolatile()) );
  }

  BOOST_TEST_CHECKPOINT("makeReservedType(..., true, true) returns cv-qualified type");
  using XcodeMl::makeReservedType;

  const auto c = makeReservedType("c", wrap("int"), true, false);
  BOOST_CHECK(c->isConst());
  BOOST_CHECK( !(c->isVolatile()) );

  const auto v = makeReservedType("v", wrap("int"), false, true);
  BOOST_CHECK( !(v->isConst()) );
  BOOST_CHECK(v->isVolatile());

  const auto cv = makeReservedType("v", wrap("int"), true, true);
  BOOST_CHECK(cv->isConst());
  BOOST_CHECK(cv->isVolatile());
}

BOOST_AUTO_TEST_CASE(RTTI_test) {
  using namespace XcodeMl;

  const auto rsv = makeReservedType("int", wrap("int"));
  const auto ptr = makePointerType("p2", "p1");
  const auto fun = makeFunctionType("f1", rsv, {});
  const auto arr = makeArrayType("a1", fun, 10);
  const auto stt = makeStructType("s1", wrap("tag"), {});

  BOOST_TEST_CHECKPOINT("isa<T1>(makeT1Type(...)) is true");

  using llvm::isa;
  BOOST_CHECK(isa<Reserved>(rsv.get()));
  BOOST_CHECK(isa<Pointer>(ptr.get()));
  BOOST_CHECK(isa<Function>(fun.get()));
  BOOST_CHECK(isa<Array>(arr.get()));
  BOOST_CHECK(isa<Struct>(stt.get()));

  BOOST_TEST_CHECKPOINT("cast<T1>(x) succeeds if x is T1");

  using llvm::cast;
  BOOST_CHECK(cast<Reserved>(rsv.get()));
  BOOST_CHECK(cast<Pointer>(ptr.get()));
  BOOST_CHECK(cast<Function>(fun.get()));
  BOOST_CHECK(cast<Array>(arr.get()));
  BOOST_CHECK(cast<Struct>(stt.get()));
}

BOOST_AUTO_TEST_SUITE_END()
