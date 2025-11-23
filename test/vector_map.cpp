#include "datagui/element/vector_map.hpp"
#include <gtest/gtest.h>
#include <random>

TEST(VectorMap, EmplaceAndPop) {
  using namespace datagui;

  VectorMap<int> map;
  int a = map.emplace(2);
  int b = map.emplace(4);

  ASSERT_TRUE(map.contains(a));
  ASSERT_TRUE(map.contains(b));
  EXPECT_EQ(map[a], 2);
  EXPECT_EQ(map[b], 4);

  map.pop(a);
  EXPECT_FALSE(map.contains(a));
  EXPECT_TRUE(map.contains(b));
}

TEST(VectorMap, String) {
  using namespace datagui;

  {
    VectorMap<std::string> map;
  }

  VectorMap<std::string> map;
  std::size_t N = 1000;
  for (std::size_t i = 0; i < N; i++) {
    map.emplace(std::to_string(i));
  }
  for (std::size_t i = 0; i < N; i += 2) {
    ASSERT_TRUE(map.contains(i));
    map.pop(i);
  }
  for (std::size_t i = 0; i < N; i++) {
    if (i % 2 == 0) {
      ASSERT_FALSE(map.contains(i));
    } else {
      ASSERT_TRUE(map.contains(i));
      EXPECT_EQ(map[i], std::to_string(i));
    }
  }
}
