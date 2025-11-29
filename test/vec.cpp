#include "datagui/geometry/vec.hpp"
#include <gtest/gtest.h>

TEST(Vec, ConstructIsZero) {
  using namespace datagui;

  Vec2 vec2;
  EXPECT_EQ(vec2.x, 0);
  EXPECT_EQ(vec2.y, 0);

  Vec3 vec3;
  EXPECT_EQ(vec3.x, 0);
  EXPECT_EQ(vec3.y, 0);
}

TEST(Vec, Add) {
  using namespace datagui;

  {
    Vec2 a = {1, 2};
    Vec2 b = {3, 4};
    Vec2 result = a + b;
    EXPECT_EQ(result.x, 4);
    EXPECT_EQ(result.y, 6);
  }

  {
    Vec3 a = {1, 2, 3};
    Vec3 b = {4, 5, 6};
    Vec3 result = a + b;
    EXPECT_EQ(result.x, 5);
    EXPECT_EQ(result.y, 6);
    EXPECT_EQ(result.z, 9);
  }
}
