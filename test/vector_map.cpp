#include "datagui/element/vector_map.hpp"
#include <gtest/gtest.h>

TEST(VectorMap, EmplaceAndPop) {
  using namespace datagui;

  VectorMap<int> value;
  int a = value.emplace(2);
  int b = value.emplace(4);

  ASSERT_TRUE(value.contains(a));
  ASSERT_TRUE(value.contains(b));
  EXPECT_EQ(value[a], 2);
  EXPECT_EQ(value[b], 4);

  value.pop(a);
  EXPECT_FALSE(value.contains(a));
  EXPECT_TRUE(value.contains(b));
}
