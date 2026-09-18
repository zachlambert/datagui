#include "datagui/render/state/scene_2d.hpp"
#include "datagui/geometry/rot.hpp"

namespace dgui {

void Scene2d::draw_rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float border_width,
    Color border_color) {
  draw_shape(
      Shape2dInstance::rect(
          position,
          angle,
          size,
          color,
          border_width,
          border_color));
}

void Scene2d::draw_circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  draw_shape(
      Shape2dInstance::circle(
          position,
          radius,
          color,
          border_width,
          border_color));
}

void Scene2d::draw_ellipse(
    const Vec2& position,
    float angle,
    const Vec2& radii,
    const Color& color,
    float border_width,
    Color border_color) {
  draw_shape(
      Shape2dInstance::ellipse(
          position,
          angle,
          radii,
          color,
          border_width,
          border_color));
}

void Scene2d::draw_line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color,
    bool rounded_ends) {
  draw_shape(Shape2dInstance::line(a, b, width, color, rounded_ends));
}

void Scene2d::draw_capsule(
    const Vec2& start,
    const Vec2& end,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  draw_shape(
      Shape2dInstance::capsule(
          start,
          end,
          radius,
          color,
          border_width,
          border_color));
}

void Scene2d::draw_shape(const Shape2dInstance& instance) {
  DrawShape* draw_shape =
      actions.empty() ? nullptr : std::get_if<DrawShape>(&actions.back());
  if (!draw_shape) {
    draw_shape = &std::get<DrawShape>(actions.emplace_back(DrawShape()));
    draw_shape->offset = shape_instances.size();
  }
  draw_shape->count++;
  shape_instances.push_back(instance);
}

void Scene2d::draw_text(
    const FontAtlas& font_atlas,
    const Vec2& origin,
    double angle,
    const Vec2& scale,
    const Color& color,
    Length width,
    const std::string& text) {
  DrawGlyph* draw_glyph =
      actions.empty() ? nullptr : std::get_if<DrawGlyph>(&actions.back());
  if (!draw_glyph || draw_glyph->font_texture != font_atlas.texture()) {
    draw_glyph = &std::get<DrawGlyph>(actions.emplace_back(DrawGlyph()));
    draw_glyph->font_texture = font_atlas.texture();
    draw_glyph->offset = glyph_instances.size();
  }
  draw_glyph->count += font_atlas.add_glyphs(
      glyph_instances,
      origin,
      angle,
      scale,
      false,
      color,
      text,
      width);
}

void Scene2d::draw_image(
    const Image& image,
    const Vec2& origin,
    double angle,
    const Vec2& scale) {
  auto& draw_image = std::get<DrawImage>(actions.emplace_back(DrawImage()));
  draw_image.transform = Mat3::transform(origin, angle, scale);
  draw_image.image = image;
}

} // namespace dgui
