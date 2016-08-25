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

BOOST_AUTO_TEST_CASE(typeKind) {
  auto reserved = XcodeMl::makeReservedType("ident1", "int");
  BOOST_CHECK(XcodeMl::typeKind(reserved) == XcodeMl::TypeKind::Reserved);
}

BOOST_AUTO_TEST_SUITE_END()
