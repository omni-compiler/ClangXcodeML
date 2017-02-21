#define BOOST_TEST_MODULE CXXCodeGen::Stream
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <sstream>

#include "CXXCodeGen.h"

namespace cxxgen = CXXCodeGen;

struct Fixture {
  cxxgen::Stream stream;
};

BOOST_FIXTURE_TEST_SUITE(cxxgen_stream, Fixture)

BOOST_AUTO_TEST_CASE(empty_string_test) {
  BOOST_TEST_CHECKPOINT(
      "Stream::str() is an empty string after initialization");

  BOOST_CHECK(stream.str().empty());
}

BOOST_AUTO_TEST_CASE(simple_string_output_test) {
  BOOST_TEST_CHECKPOINT("Stream accepts a string");
  stream << "string";
  BOOST_CHECK(stream.str() == "string");
}

BOOST_AUTO_TEST_SUITE_END()
