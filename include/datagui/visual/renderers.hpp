#pragma once

#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct Renderers {
  GeometryRenderer geometry;
  TextRenderer text;

  Renderers(FontManager& font_manager) : text(font_manager) {}
};

} // namespace datagui
