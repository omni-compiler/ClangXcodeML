#define BOOST_TEST_MODULE CXXCodeGen::Stream
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <sstream>

#include "CXXCodeGen.h"

namespace cxxgen = CXXCodeGen;

BOOST_AUTO_TEST_SUITE(cxxgen_stream)

BOOST_AUTO_TEST_CASE(empty_string_test) {
  BOOST_TEST_CHECKPOINT(
      "Stream::str() is an empty string after initialization");

  cxxgen::Stream stream;
  BOOST_CHECK(stream.str().empty());
}

BOOST_AUTO_TEST_SUITE_END()
