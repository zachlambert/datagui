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

} // namespace datagui
