#define BOOST_TEST_MODULE CXXCodeGen::Stream
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <sstream>
#include <tuple>
#include <utility>

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

BOOST_AUTO_TEST_CASE(newline_output_test) {
  BOOST_TEST_CHECKPOINT("Stream accepts a newline_t");
  stream << cxxgen::newline;
  BOOST_CHECK(stream.str() == "\n");
}

BOOST_AUTO_TEST_CASE(space_output_test) {
  BOOST_TEST_CHECKPOINT("Stream accepts a space_t");
  stream << cxxgen::space;
  BOOST_CHECK(stream.str() == " ");
}

BOOST_AUTO_TEST_CASE(char_output_test) {
  BOOST_TEST_CHECKPOINT("Stream accepts a char");
  const auto chars = static_cast<std::string>(" 01aA;\n\t");
  for (char c : chars) {
    const std::string str = {c};
    BOOST_TEST_MESSAGE("Checking '" + str + "'");
    cxxgen::Stream model, learner;
    model << str, learner << c;
    BOOST_CHECK(model.str() == learner.str());
  }
}

BOOST_AUTO_TEST_CASE(space_redundancy_test) {
  BOOST_TEST_CHECKPOINT("Stream does not emit redundant space");

  const std::vector<std::pair<char,std::string>> seps = {
    { '\n', "newline"},
    { ' ', "space"},
    { '\t', "tab" },
  };

  for (auto&& sep : seps) {
    BOOST_TEST_MESSAGE("Checking " + sep.second);
    const char s = sep.first;
    cxxgen::Stream model, learner;

    model << s, learner << s;
    learner << cxxgen::space;

    BOOST_CHECK(model.str() == learner.str());
  }
}

BOOST_AUTO_TEST_SUITE_END()
