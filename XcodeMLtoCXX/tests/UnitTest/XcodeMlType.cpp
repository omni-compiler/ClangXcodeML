#define BOOST_TEST_MODULE XcodeMl::Type
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <string>
#include <vector>
#include <libxml/tree.h>
#include "SymbolAnalyzer.h"
#include "XcodeMlType.h"
#include "XcodeMlEnvironment.h"
#include "TypeAnalyzer.h"
#include "XcodeMlType.h"

BOOST_AUTO_TEST_SUITE(xcodeml_type)

BOOST_AUTO_TEST_CASE(makeXXXType_nullability_test) {
  BOOST_TEST_CHECKPOINT("makeXXXType(...) is not nullable");

  auto intType = XcodeMl::makeReservedType("int", "int");
  BOOST_CHECK(intType);

  BOOST_CHECK(XcodeMl::makePointerType("p1", intType));
  BOOST_CHECK(XcodeMl::makePointerType("p2", "p1"));

  BOOST_CHECK(XcodeMl::makeFunctionType("f1", intType, {}));

  BOOST_CHECK(XcodeMl::makeArrayType("a1", intType, 10));

  BOOST_CHECK(XcodeMl::makeStructType("s1", "name", "tag", {}));
}

BOOST_AUTO_TEST_CASE(makeXXXType_attr_test) {
  BOOST_TEST_CHECKPOINT("Type of `makeXXXType(...)` == XXX");

  using XcodeMl::TypeKind;
  using XcodeMl::typeKind;

  const auto reserved = XcodeMl::makeReservedType("ident1", "int");
  BOOST_CHECK(typeKind(reserved) == TypeKind::Reserved);

  const auto pointer1 = XcodeMl::makePointerType("p1", reserved);
  BOOST_CHECK(typeKind(pointer1) == TypeKind::Pointer);

  const auto pointer2 = XcodeMl::makePointerType("p2", "ident1");
  BOOST_CHECK(typeKind(pointer2) == TypeKind::Pointer);

  const auto function = XcodeMl::makeFunctionType("f1", reserved, {});
  BOOST_CHECK(typeKind(function) == TypeKind::Function);

  const auto structure = XcodeMl::makeStructType("s1", "name", "tag", {});
  BOOST_CHECK(typeKind(structure) == TypeKind::Struct);


  BOOST_TEST_CHECKPOINT("XcodeMl::Type::dataTypeIdent()");

  BOOST_CHECK_EQUAL(reserved->dataTypeIdent(), "ident1");
  BOOST_CHECK_EQUAL(pointer1->dataTypeIdent(), "p1");
  BOOST_CHECK_EQUAL(pointer2->dataTypeIdent(), "p2");
  BOOST_CHECK_EQUAL(function->dataTypeIdent(), "f1");
  BOOST_CHECK_EQUAL(structure->dataTypeIdent(), "s1");
}

BOOST_AUTO_TEST_CASE(cv_qualification_test) {
  BOOST_TEST_CHECKPOINT("makeXXXType(...) returns cv-unqualified type");
  auto rsv1 = XcodeMl::makeReservedType("rsv1", "int");
  std::vector<XcodeMl::TypeRef> types = {
    rsv1,
    XcodeMl::makeReservedType("rsv2", "rsv1"),
    XcodeMl::makePointerType("ptr1", "rsv2"),
    XcodeMl::makePointerType("ptr1", rsv1),
    XcodeMl::makeFunctionType("fun1", rsv1, {}),
    XcodeMl::makeArrayType("arr1", rsv1, 10),
    XcodeMl::makeStructType("str1", "name", "tag", {})
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

  const auto c = makeReservedType("c", "int", true, false);
  BOOST_CHECK(c->isConst());
  BOOST_CHECK( !(c->isVolatile()) );

  const auto v = makeReservedType("v", "int", false, true);
  BOOST_CHECK( !(v->isConst()) );
  BOOST_CHECK(v->isVolatile());

  const auto cv = makeReservedType("v", "int", true, true);
  BOOST_CHECK(cv->isConst());
  BOOST_CHECK(cv->isVolatile());
}

BOOST_AUTO_TEST_SUITE_END()
