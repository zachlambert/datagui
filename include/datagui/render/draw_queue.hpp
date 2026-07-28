#pragma once

#include "datagui/asset/image.hpp"
#include "datagui/render/draw_list.hpp"
#include "datagui/render/font_atlas.hpp"

namespace dgui {

struct DrawQueue {
public:
  void begin_group(const Box2& mask);
  void begin_scene_2d(const Box2& viewport, const Camera2d& camera);

  void draw_box(
      const Box2& box,
      const Color& color,
      float border_width,
      Color border_color,
      float radius);

  void draw_text(
      const FontAtlas& font_atlas,
      const Vec2& origin,
      double angle,
      const Vec2& scale,
      const Color& color,
      Length width,
      const std::string& text);

  void draw_image(
      const Image& image,
      const Vec2& origin,
      double angle,
      Vec2& scale);

  DrawList flush();

private:
  void draw_shape_2d(const Shape2dInstance& instance);
  void clear_active();
  DrawList::DrawShape2d& draw_shape_2d() {
    return std::get<DrawList::DrawShape2d>(dl.actions[draw_shape_2d_i]);
  }
  DrawList::DrawGlyph2d& draw_glyph_2d() {
    return std::get<DrawList::DrawGlyph2d>(dl.actions[draw_glyph_2d_i]);
  }

  DrawList dl;
  bool keep_order = false;
  bool y_flipped = true;
  int draw_shape_2d_i = -1;
  int draw_glyph_2d_i = -1;
};

} // namespace dgui
