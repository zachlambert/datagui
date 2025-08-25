#pragma once

#include "datagui/style.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct Resources {
  FontManager font_manager;
  StyleManager style_manager;
  GeometryRenderer geometry_renderer;
  TextRenderer text_renderer;

  Resources() : text_renderer(font_manager) {}
};

} // namespace datagui
