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
  Length left, right, up, down;

  BoxDims() :
      left(LengthWrap()),
      right(LengthWrap()),
      up(LengthWrap()),
      down(LengthWrap()) {}

  BoxDims(
      const Length& left,
      const Length& right,
      const Length& up,
      const Length& down) :
      left(left), right(right), up(up), down(down) {}
};

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
