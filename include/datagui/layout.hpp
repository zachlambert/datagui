#pragma once

#include "datagui/geometry.hpp"
#include <variant>
#include <vector>

namespace datagui {

enum class XAlignment { Left, Center, Right };
enum class YAlignment { Top, Center, Bottom };
enum class Direction { Vertical, Horizontal };

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

struct Layout {
  int rows = 0;
  int cols = 0;
  XAlignment x_alignment = XAlignment::Left;
  YAlignment y_alignment = YAlignment::Center;
  bool tight = false;
};

struct InputSizes {
  float fixed;
  float dynamic;
};

struct LayoutState {
  std::vector<InputSizes> row_input_sizes;
  std::vector<InputSizes> col_input_sizes;

  Vec2 content_fixed_size;
  Vec2 content_dynamic_size;
  Vec2 content_overrun;
  Vec2 scroll_pos;
};

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

  Vec2 size() const {
    return Vec2(left + right, top + bottom);
  }
  Vec2 offset() const {
    return Vec2(left, top);
  }
  Vec2 offset_opposite() const {
    return Vec2(right, bottom);
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
  Vec2 size;
  FloatingTypeAbsolute(const Vec2& size) : size(size) {}
};

struct FloatingTypeRelative {
  Vec2 offset;
  Vec2 size;
  FloatingTypeRelative(const Vec2& offset, const Vec2& size) :
      offset(offset), size(size) {}
};

using FloatingType = std::variant<FloatingTypeAbsolute, FloatingTypeRelative>;

} // namespace datagui
