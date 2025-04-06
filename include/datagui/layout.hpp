#pragma once

#include "datagui/geometry.hpp"
#include <variant>

namespace datagui {

enum class Direction { Horizontal, Vertical };

struct LengthFixed {
  float value = 0;
  LengthFixed() = default;
  LengthFixed(float value) : value(value) {}
};

struct LengthDynamic {
  float weight = 0;
  LengthDynamic() = default;
  LengthDynamic(float weight) : weight(weight) {}
};

struct LengthWrap {};

using Length = std::variant<LengthFixed, LengthDynamic, LengthWrap>;

namespace literals {

static constexpr Length _wrap = LengthWrap();
inline Length operator""_fixed(long double value) {
  return LengthFixed(value);
}
inline Length operator""_dynamic(long double weight) {
  return LengthDynamic(weight);
}

} // namespace literals

class BoxDims {
public:
  float left, right, top, bottom;

  BoxDims() : left(0), right(0), top(0), bottom(0) {}

  BoxDims(float value) : left(value), right(value), top(value), bottom(value) {}

  BoxDims(float horizontal, float vertical) :
      left(horizontal), right(horizontal), top(vertical), bottom(vertical) {}

  // Use same ordering as CSS
  BoxDims(float left, float top, float right, float bottom) :
      left(left), right(right), top(top), bottom(bottom) {}

  Vecf size() const {
    return Vecf(left + right, top + bottom);
  }
  Vecf offset() const {
    return Vecf(left, top);
  }

  BoxDims& operator+=(const BoxDims& rhs) {
    left += rhs.left;
    right += rhs.right;
    top += rhs.top;
    bottom += rhs.bottom;
    return *this;
  }
  BoxDims& operator-=(const BoxDims& rhs) {
    left -= rhs.left;
    right -= rhs.right;
    top -= rhs.top;
    bottom -= rhs.bottom;
    return *this;
  }
  BoxDims& operator*=(float rhs) {
    left *= rhs;
    right *= rhs;
    top *= rhs;
    bottom *= rhs;
    return *this;
  }
  BoxDims& operator/=(float rhs) {
    left /= rhs;
    right /= rhs;
    top /= rhs;
    bottom /= rhs;
    return *this;
  }
};

inline BoxDims operator+(BoxDims lhs, const BoxDims& rhs) {
  lhs += rhs;
  return lhs;
}

inline BoxDims operator-(BoxDims lhs, const BoxDims& rhs) {
  lhs -= rhs;
  return lhs;
}

inline BoxDims operator*(BoxDims lhs, float rhs) {
  lhs *= rhs;
  return lhs;
}

inline BoxDims operator/(BoxDims lhs, float rhs) {
  lhs /= rhs;
  return lhs;
}

struct LayoutSize {
  Vecf fixed;
  Vecf dynamic;

  LayoutSize& operator+=(const LayoutSize& rhs) {
    fixed += rhs.fixed;
    dynamic += rhs.dynamic;
    return *this;
  }
  LayoutSize& operator-=(const LayoutSize& rhs) {
    fixed -= rhs.fixed;
    dynamic -= rhs.dynamic;
    return *this;
  }
  LayoutSize& operator*=(float rhs) {
    fixed *= rhs;
    dynamic *= rhs;
    return *this;
  }
  LayoutSize& operator/=(float rhs) {
    fixed /= rhs;
    dynamic /= rhs;
    return *this;
  }
  LayoutSize operator-() {
    LayoutSize result;
    result.fixed = -fixed;
    result.dynamic = -dynamic;
    return result;
  };
};

inline LayoutSize operator+(LayoutSize lhs, const LayoutSize& rhs) {
  lhs += rhs;
  return lhs;
}

inline LayoutSize operator-(LayoutSize lhs, const LayoutSize& rhs) {
  lhs -= rhs;
  return lhs;
}

inline LayoutSize operator*(LayoutSize lhs, float rhs) {
  lhs *= rhs;
  return lhs;
}

inline LayoutSize operator/(LayoutSize lhs, float rhs) {
  lhs /= rhs;
  return lhs;
}

} // namespace datagui
