#pragma once

#include "datagui/geometry/camera.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"
#include "datagui/render/state/shape_2d_instance.hpp"
#include <variant>

namespace dgui {

struct DrawList {
  std::vector<Shape2dInstance> shape_2d_instances;
  std::vector<Glyph2dInstance> glyph_2d_instances;

  struct BeginGroup {
    Box2 mask;
  };
  struct BeginScene2d {
    Box2 viewport;
    Camera2d camera;
  };
  struct DrawShape2d {
    size_t offset;
    size_t count;
  };
  struct DrawGlyph2d {
    unsigned int font_texture;
    size_t offset;
    size_t count;
  };
  struct DrawImage2d {
    unsigned int image_texture;
    Mat3 transform;
  };

  using Action = std::variant<
      BeginGroup,
      BeginScene2d,
      DrawShape2d,
      DrawGlyph2d,
      DrawImage2d>;
  std::vector<Action> actions;
};

} // namespace dgui
