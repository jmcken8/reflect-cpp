#include <gtest/gtest.h>

#include <iostream>
#include <rfl.hpp>
#include <rfl/cbor.hpp>
#include <rfl/json.hpp>
#include <string>
#include <vector>

namespace test_error_messages {

struct Person {
  rfl::Rename<"firstName", std::string> first_name;
  rfl::Rename<"lastName", std::string> last_name;
  rfl::Timestamp<"%Y-%m-%d"> birthday;
  std::vector<Person> children;
};

TEST(cbor, test_field_error_messages) {
  const std::string faulty_string =
      R"({"firstName":"Homer","lastName":12345,"birthday":"04/19/1987"})";
  const auto faulty_generic = rfl::json::read<rfl::Generic>(faulty_string);
  const auto faulty_cbor = rfl::cbor::write(faulty_generic);
  const auto result = rfl::cbor::read<Person>(faulty_cbor);

  // Order of errors is different than input JSON because rfl::Generic doesn't preserve order
  const std::string expected = R"(Found 3 errors:
1) Failed to parse field 'birthday': String '04/19/1987' did not match format '%Y-%m-%d'.
2) Failed to parse field 'lastName': Could not cast to string.
3) Field named 'children' not found.)";

  EXPECT_TRUE(!result.has_value() && true);

  EXPECT_EQ(result.error().what(), expected);
}

TEST(cbor, test_decode_error_without_exception) {
  const std::string good_string =
      R"({"firstName":"Homer","lastName":"Simpson","birthday":"1987-04-19"})";
  const auto good_generic = rfl::json::read<rfl::Generic>(good_string);
  auto faulty_cbor = rfl::cbor::write(good_generic);
  faulty_cbor[1] = '\xff';  // Corrupt structure of CBOR encoding

  rfl::Result<Person> result = rfl::error("result didn't get set");
  EXPECT_NO_THROW({
    result = rfl::cbor::read<Person>(faulty_cbor);
  });

  // A proposal: A generic prefix, followed by the underlying library's error output
    const std::string expected = R"(Found 4 errors:
1) Field named 'firstName' not found.
2) Field named 'lastName' not found.
3) Field named 'birthday' not found.
4) Field named 'children' not found.)";
  EXPECT_EQ(result.error().what(), expected);

  EXPECT_TRUE(!result.has_value() && true);
}

}  // namespace test_error_messages
