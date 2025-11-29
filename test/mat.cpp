#include "datagui/geometry/mat.hpp"
#include <gtest/gtest.h>

TEST(Mat, ConstructIsZero) {
  using namespace datagui;

  Mat2 mat2;
  for (std::size_t i = 0; i < 2; i++) {
    for (std::size_t j = 0; j < 2; j++) {
      EXPECT_EQ(mat2(i, j), 0);
    }
  }

  Mat3 mat3;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      EXPECT_EQ(mat3(i, j), 0);
    }
  }

  Mat4 mat4;
  for (std::size_t i = 0; i < 4; i++) {
    for (std::size_t j = 0; j < 4; j++) {
      EXPECT_EQ(mat4(i, j), 0);
    }
  }
}

TEST(Mat, VectorMultiply) {
  using namespace datagui;

  {
    Mat2 A = {1, 2, 3, 4};
    Vec2 b = {5, 10};
    Vec2 c = A * b;
    EXPECT_EQ(c.x, 25);
    EXPECT_EQ(c.y, 55);
  }

  {
    Mat3 A = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    Vec3 b = {1, 2, 3};
    Vec3 c = A * b;
    EXPECT_EQ(c.x, 14);
    EXPECT_EQ(c.y, 32);
    EXPECT_EQ(c.z, 50);
  }
}
