#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/rot.hpp"

namespace datagui {

Mat4 Mat4::Transform(const Vec3& position, const Rot3& orientation) {
  Mat4 result;
  result(3, 3) = 1;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      result(i, j) = orientation.mat()(i, j);
    }
  }
  for (std::size_t i = 0; i < 3; i++) {
    result(i, 3) = position(i);
  }
  return result;
}

Mat4 Mat4::Transform(
    const Vec3& position,
    const Rot3& orientation,
    const Vec3& scale) {
  Mat3 scale_mat;
  for (std::size_t i = 0; i < 3; i++) {
    scale_mat(i, i) = scale(i);
  }
  Mat3 top_left = orientation.mat() * scale_mat;

  Mat4 result;
  result(3, 3) = 1;
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      result(i, j) = top_left(i, j);
    }
  }
  for (std::size_t i = 0; i < 3; i++) {
    result(i, 3) = position(i);
  }
  return result;
}

#ifdef DATAGUI_DEBUG

std::ostream& operator<<(std::ostream& os, const Mat2& mat) {
  for (std::size_t i = 0; i < 2; i++) {
    for (std::size_t j = 0; j < 2; j++) {
      os << mat(i, j);
      if (j + 1 != 2) {
        os << ", ";
      }
    }
    if (i + 1 != 2) {
      os << "\n";
    }
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Mat3& mat) {
  for (std::size_t i = 0; i < 3; i++) {
    for (std::size_t j = 0; j < 3; j++) {
      os << mat(i, j);
      if (j + 1 != 3) {
        os << ", ";
      }
    }
    if (i + 1 != 3) {
      os << "\n";
    }
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Mat4& mat) {
  for (std::size_t i = 0; i < 4; i++) {
    for (std::size_t j = 0; j < 4; j++) {
      os << mat(i, j);
      if (j + 1 != 4) {
        os << ", ";
      }
    }
    if (i + 1 != 4) {
      os << "\n";
    }
  }
  return os;
}

#endif

} // namespace datagui
