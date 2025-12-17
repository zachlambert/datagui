#include "datagui/geometry/mat.hpp"

namespace datagui {

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
