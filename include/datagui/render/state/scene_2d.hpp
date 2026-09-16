#pragma once

#include "datagui/asset/image.hpp"
#include "datagui/render/font_atlas.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"
#include "datagui/render/state/shape_2d_instance.hpp"
#include <variant>

namespace dgui {

struct Scene2d {
  struct DrawShape {
    size_t offset;
    size_t count;
  };
  struct DrawGlyph {
    unsigned int font_texture;
    size_t offset;
    size_t count;
  };
  struct DrawImage {
    Image image;
    Mat3 transform;
  };
  using Action = std::variant<DrawShape, DrawGlyph, DrawImage>;

  Color bg_color = Color::White();
  std::vector<Shape2dInstance> shape_instances;
  std::vector<Glyph2dInstance> glyph_instances;
  std::vector<Action> actions;

  void clear() {
    bg_color = Color::White();
    shape_instances.clear();
    glyph_instances.clear();
    actions.clear();
  }

  void draw_rect(
      const Vec2& position,
      float angle,
      const Vec2& size,
      const Color& color,
      float border_width = 0,
      Color border_color = Color::Black());

  void draw_circle(
      const Vec2& position,
      float radius,
      const Color& color,
      float border_width = 0,
      Color border_color = Color::Black());

  void draw_ellipse(
      const Vec2& position,
      float angle,
      const Vec2& radii,
      const Color& color,
      float border_width = 0,
      Color border_color = Color::Black());

  void draw_line(
      const Vec2& a,
      const Vec2& b,
      float width,
      const Color& color,
      bool rounded_ends = false);

  void draw_capsule(
      const Vec2& start,
      const Vec2& end,
      float radius,
      const Color& color,
      float border_width = 0,
      Color border_color = Color::Black());

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
      const Vec2& scale);

private:
  void draw_shape(const Shape2dInstance& instance);
};

} // namespace dgui
