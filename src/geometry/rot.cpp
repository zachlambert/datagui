#include "datagui/geometry/rot.hpp"
#include <array>

namespace datagui {

Rot3::Rot3(const Euler& euler) {
  float s1 = std::sin(euler.roll);
  float s2 = std::sin(euler.pitch);
  float s3 = std::sin(euler.yaw);
  float c1 = std::cos(euler.roll);
  float c2 = std::cos(euler.pitch);
  float c3 = std::cos(euler.yaw);

  // clang-format off
  mat_ = {
	  c2*c3, s1*s2*c3 - c1*s3, c1*s2*c3 + s1*s3,
	  c2*s3, s1*s2*s3 + c1*c3, c1*s2*s3 - s1*c3,
	    -s2,            s1*c2,            c1*c2
  };
  mat_ = mat_.transpose(); // Defined in column-major order
  // clang-format on
}

Rot3::Rot3(const Quat& quat) {
  for (std::size_t i = 0; i < 3; i++) {
    mat_(i, i) = 2 * (std::pow(quat.data[i], 2) + std::pow(quat.w, 2)) - 1;
  }

  std::array<std::size_t, 3> permutations[3] = {
      {0, 1, 2},
      {0, 2, 1},
      {1, 2, 0}};
  int signs[3] = {-1, 1, -1};

  for (std::size_t index = 0; index < 3; index++) {
    auto [i, j, k] = permutations[index];
    int sign = signs[index];
    // clang-format off
    mat_(i, j) = 2 * (quat.data[i] * quat.data[j] + sign * quat.w * quat.data[k]);
    mat_(j, i) = 2 * (quat.data[i] * quat.data[j] - sign * quat.w * quat.data[k]);
    // clang-format on
  }
}

Rot2 Rot2::line_rot(const Vec2& start, const Vec2& end) {
  float angle = std::atan2(end.y - start.y, end.x - start.x);
  return Rot2(angle);
}

Rot3 Rot3::line_rot(const Vec3& start, const Vec3& end) {
  Vec3 disp = end - start;
  float length = disp.length();
  if (length < 1e-6) {
    return Rot3();
  }
  Vec3 u1 = disp / length;
  return line_rot(disp / length);
}

Rot3 Rot3::line_rot(const Vec3& direction) {
  const Vec3& u1 = direction;

  std::size_t min_axis = 0;
  for (std::size_t i = 1; i < 3; i++) {
    if (u1(i) < std::fabs(u1(min_axis))) {
      min_axis = i;
    }
  }
  Vec3 cross_with;
  cross_with(min_axis) = 1.f;

  Vec3 u2 = u1.cross(cross_with);
  u2 /= u2.length();
  Vec3 u3 = u1.cross(u2);

  return Mat3(u1, u2, u3);
}

} // namespace datagui
