#define BOOST_TEST_MODULE XcodeMl::Type
#include <boost/test/included/unit_test.hpp>
#include <memory>
#include <string>
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

BOOST_AUTO_TEST_SUITE_END()
