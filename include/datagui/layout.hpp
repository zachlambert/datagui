#pragma once

#include "datagui/geometry.hpp"
#include <variant>

namespace datagui {

enum class Direction { Horizontal, Vertical };
enum class Alignment { Min, Center, Max };

struct LengthFixed {
  float value = 0;
  LengthFixed() = default;
  LengthFixed(float value) : value(value) {}
};

struct LengthDynamic {
  float weight = 1;
  LengthDynamic() = default;
  LengthDynamic(float weight) : weight(weight) {}
};

struct LengthWrap {};

using Length = std::variant<LengthFixed, LengthDynamic, LengthWrap>;

class BoxDims {
public:
  // Use same ordering as CSS
  // Also makes the layout easier for geometry_renderer shader
  float left, top, right, bottom;

  BoxDims() : left(0), top(0), right(0), bottom(0) {}

  BoxDims(float value) : left(value), top(value), right(value), bottom(value) {}

  BoxDims(float horizontal, float vertical) :
      left(horizontal), top(vertical), right(horizontal), bottom(vertical) {}

  BoxDims(float left, float top, float right, float bottom) :
      left(left), top(top), right(right), bottom(bottom) {}

  Vecf size() const {
    return Vecf(left + right, top + bottom);
  }
  Vecf offset() const {
    return Vecf(left, top);
  }
  Vecf offset_opposite() const {
    return Vecf(right, bottom);
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

struct FloatingTypeAbsolute {
  BoxDims margin;
  FloatingTypeAbsolute(const BoxDims& margin) : margin(margin) {}
};

struct FloatingTypeRelative {
  Vecf offset;
  Vecf size;
  FloatingTypeRelative(const Vecf& offset, const Vecf& size) :
      offset(offset), size(size) {}
};

using FloatingType = std::variant<FloatingTypeAbsolute, FloatingTypeRelative>;

} // namespace datagui
