#include "datagui/geometry/rot.hpp"
#include <gtest/gtest.h>

TEST(Quat, ConstructIsIdentity) {
  using namespace datagui;

  Quat q;
  EXPECT_EQ(q.x, 0);
  EXPECT_EQ(q.y, 0);
  EXPECT_EQ(q.z, 0);
  EXPECT_EQ(q.w, 1);
}

TEST(Euler, ConstructIsZero) {
  using namespace datagui;

  Euler euler;
  EXPECT_EQ(euler.roll, 0);
  EXPECT_EQ(euler.pitch, 0);
  EXPECT_EQ(euler.yaw, 0);
}

TEST(Rot, ConstructIsIdentity) {
  using namespace datagui;

  Rot2 rot2;
  for (std::size_t i = 0; i < 2; i++) {
    for (std::size_t j = 0; j < 2; j++) {
      if (i == j) {
        EXPECT_EQ(rot2.mat()(i, j), 1);
      } else {
        EXPECT_EQ(rot2.mat()(i, j), 0);
      }
    }
  }

  Rot3 rot3;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      if (i == j) {
        EXPECT_EQ(rot3.mat()(i, j), 1);
      } else {
        EXPECT_EQ(rot3.mat()(i, j), 0);
      }
    }
  }
}

TEST(Rot, ConstructRot2FromAngle) {
  using namespace datagui;
  const float eps = 1e-6;

  {
    Rot2 rot = Rot2(M_PI / 2);
    EXPECT_NEAR(rot.mat()(0, 0), 0.f, eps);
    EXPECT_NEAR(rot.mat()(1, 0), 1.f, eps);
    EXPECT_NEAR(rot.mat()(0, 1), -1.f, eps);
    EXPECT_NEAR(rot.mat()(1, 1), 0.f, eps);
  }
  {
    Rot2 rot = Rot2(M_PI);
    EXPECT_NEAR(rot.mat()(0, 0), -1.f, eps);
    EXPECT_NEAR(rot.mat()(1, 0), 0.f, eps);
    EXPECT_NEAR(rot.mat()(0, 1), 0.f, eps);
    EXPECT_NEAR(rot.mat()(1, 1), -1.f, eps);
  }
  {
    Rot2 rot = Rot2(M_PI / 4);
    EXPECT_NEAR(rot.mat()(0, 0), 1.f / std::sqrt(2.f), eps);
    EXPECT_NEAR(rot.mat()(1, 0), 1.f / std::sqrt(2.f), eps);
    EXPECT_NEAR(rot.mat()(0, 1), -1.f / std::sqrt(2.f), eps);
    EXPECT_NEAR(rot.mat()(1, 1), 1.f / std::sqrt(2.f), eps);
  }
}

TEST(Rot, ConstructRot3FromEuler) {
  using namespace datagui;
  const float eps = 1e-5;

  {
    Rot3 rot = Rot3(Euler(-M_PI / 2, M_PI / 2, M_PI / 2));

    // X -> -Z
    EXPECT_NEAR(rot.mat()(0, 0), 0.f, eps);
    EXPECT_NEAR(rot.mat()(1, 0), 0.f, eps);
    EXPECT_NEAR(rot.mat()(2, 0), -1.f, eps);

    // Y -> -Y
    EXPECT_NEAR(rot.mat()(0, 1), 0.f, eps);
    EXPECT_NEAR(rot.mat()(1, 1), -1.f, eps);
    EXPECT_NEAR(rot.mat()(2, 1), 0.f, eps);

    // Z -> -X
    EXPECT_NEAR(rot.mat()(0, 2), -1.f, eps);
    EXPECT_NEAR(rot.mat()(1, 2), 0.f, eps);
    EXPECT_NEAR(rot.mat()(2, 2), 0.f, eps);
  }
}

TEST(Rot, ConstructRot3FromQuat) {
  using namespace datagui;
  const float eps = 1e-5;

  {
    // Rotate about +Y by -pi/4
    Quat quat;
    quat.w = std::cos(-(M_PI / 4) / 2);
    quat.y = std::sin(-(M_PI / 4) / 2);

    Rot3 rot = Rot3(quat);

    // X -> (1/sqrt(2), 0, 1/sqrt(2))
    EXPECT_NEAR(rot.mat()(0, 0), 1.f / std::sqrt(2.f), eps);
    EXPECT_NEAR(rot.mat()(1, 0), 0.f, eps);
    EXPECT_NEAR(rot.mat()(2, 0), 1.f / std::sqrt(2.f), eps);

    // Y -> +Z (unchanged)
    EXPECT_NEAR(rot.mat()(0, 1), 0.f, eps);
    EXPECT_NEAR(rot.mat()(1, 1), 1.f, eps);
    EXPECT_NEAR(rot.mat()(2, 1), 0.f, eps);

    // Z -> (-1/sqrt(2), 0, 1/sqrt(2))
    EXPECT_NEAR(rot.mat()(0, 2), -1.f / std::sqrt(2.f), eps);
    EXPECT_NEAR(rot.mat()(1, 2), 0.f, eps);
    EXPECT_NEAR(rot.mat()(2, 2), 1.f / std::sqrt(2.f), eps);
  }
}
