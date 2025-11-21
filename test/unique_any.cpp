#include "datagui/tree/unique_any.hpp"
#include <gtest/gtest.h>

TEST(UniqueAny, Construct) {
  using namespace datagui;

  UniqueAny value;
  EXPECT_FALSE(value);

  value = UniqueAny::Make<std::string>("hello");
  EXPECT_TRUE(value.cast<std::string>());
  EXPECT_EQ(*value.cast<std::string>(), "hello");
  EXPECT_FALSE(value.cast<int>());
}
