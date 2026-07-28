#include "datagui/render/draw_queue.hpp"

namespace dgui {

void DrawQueue::begin_group(const Box2& mask) {
  dl.actions.push_back(DrawList::BeginGroup{mask});
  clear_active();
  y_flipped = true;
  keep_order = false;
}

void DrawQueue::begin_scene_2d(const Box2& viewport, const Camera2d& camera) {
  dl.actions.push_back(DrawList::BeginScene2d{viewport, camera});
  clear_active();
  y_flipped = false;
  keep_order = true;
}

void DrawQueue::draw_shape_2d(const Shape2dInstance& instance) {
  if (draw_shape_2d_i < 0) {
    dl.actions.push_back(
        DrawList::DrawShape2d{dl.shape_2d_instances.size(), 0});
    if (keep_order) {
      clear_active();
    }
    draw_shape_2d_i = dl.actions.size() - 1;
  }
  draw_shape_2d().count++;
  dl.shape_2d_instances.push_back(instance);
}

void DrawQueue::draw_box(
    const Box2& box,
    const Color& color,
    float border_width,
    Color border_color,
    float radius) {
  draw_shape_2d(
      Shape2dInstance::box(box, color, border_width, border_color, radius));
}

void DrawQueue::draw_text(
    const FontAtlas& font_atlas,
    const Vec2& origin,
    double angle,
    const Vec2& scale,
    const Color& color,
    Length width,
    const std::string& text) {
  if (draw_glyph_2d_i < 0 ||
      draw_glyph_2d().font_texture != font_atlas.texture()) {
    dl.actions.push_back(
        DrawList::DrawGlyph2d{
            font_atlas.texture(),
            dl.glyph_2d_instances.size(),
            0});
    if (keep_order) {
      clear_active();
    }
    draw_glyph_2d_i = dl.actions.size() - 1;
  }
  size_t start = draw_glyph_2d().offset;
  draw_glyph_2d().count += font_atlas.add_glyphs(
      dl.glyph_2d_instances,
      origin,
      angle,
      scale,
      y_flipped,
      color,
      text,
      width);
}

void DrawQueue::draw_image(
    const Image& image,
    const Vec2& origin,
    double angle,
    Vec2& scale) {
  Mat3 transform = Mat3::transform(origin, Rot2(angle), scale);
  dl.actions.push_back(DrawList::DrawImage2d{image.texture(), transform});
  if (keep_order) {
    clear_active();
  }
}

DrawList DrawQueue::flush() {
  keep_order = false;
  y_flipped = false;
  clear_active();
  return std::move(dl);
}

void DrawQueue::clear_active() {
  draw_shape_2d_i = -1;
  draw_glyph_2d_i = -1;
}

} // namespace dgui
