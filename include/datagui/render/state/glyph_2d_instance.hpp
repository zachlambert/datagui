#pragma once

#include "datagui/geometry/mat.hpp"
#include "datagui/geometry/vec.hpp"

namespace dgui {

struct Glyph2dInstance {
  Mat3 M;
  Vec2 uv_offset;
  Vec2 uv_size;
};

} // namespace dgui
