#include "datagui/render/draw_builder.hpp"

namespace dgui {

void DrawBuilder::begin_group(int z_index, const Box2& mask) {
  add_batch(z_index, mask);
  in_scene_2d_ = false;
}

void DrawBuilder::begin_scene_2d(
    int z_index,
    const Color& bg_color,
    const Box2& mask) {
  add_batch(z_index, mask);
  in_scene_2d_ = true;
  prev_call_type_.reset();
}

void DrawBuilder::queue_scene_3d(
    int z_index,
    const Color& bg_color,
    const Box2& mask,
    Scene3d* scene_3d) {
  add_batch(z_index, mask);
  queue_box(mask, bg_color, 0, Color::Black(), 0);
  in_scene_2d_ = false;
}

DrawList DrawBuilder::flush() {
  return std::move(dl);
}

void DrawBuilder::add_batch(int z_index, const Box2& mask) {
  batch_z_index = z_index;
  auto& batch_list = dl.batches.try_emplace(z_index).first->second;
  auto& batch = batch_list.emplace_back();
  batch.mask = mask;
  batch.shape_count = dl.shape_2d_instances.size();
  this->batch = &batch;
}

void DrawBuilder::queue_shape_2d() {
  if (in_scene_2d_ &&
      (prev_call_type_ && *prev_call_type_ != CallType::Geometry)) {
    add_batch(batch_z_index, batch->mask);
  }
  prev_call_type_ = CallType::Geometry;
  batch->shape_count++;
}

void DrawBuilder::queue_text(
    const Vec2& origin,
    double angle,
    const Vec2& scale,
    Font font,
    int font_size,
    const Color& color,
    Length width,
    const std::string& text) {
  if (in_scene_2d_ && (prev_call_type_ && *prev_call_type_ != CallType::Text)) {
    add_batch(batch_z_index, batch->mask);
  }
  prev_call_type_ = CallType::Text;

  auto& fs = fm->font_structure(font, font_size);

  if (batch->glyph_groups.empty() ||
      !batch->glyph_groups.back().matches(fs.font_texture, color)) {
    auto& group = batch->glyph_groups.emplace_back();
    group.font_texture = fs.font_texture;
    group.color = color;
    group.offset = dl.glyph_2d_vertices.size();
  }
  auto& group = batch->glyph_groups.back();

  auto characters = fm->text_characters(text, font, font_size, width);
  auto& vertices = dl.glyph_2d_vertices;
  const size_t initial_vertex_count = vertices.size();

  for (auto& [box, uv] : characters) {
    Mat2 rot = Rot2(angle).mat();
    Vec2 lower_left = origin + scale * (rot * box.lower_left());
    Vec2 lower_right = origin + scale * (rot * box.lower_right());
    Vec2 upper_left = origin + scale * (rot * box.upper_left());
    Vec2 upper_right = origin + scale * (rot * box.upper_right());

    vertices.emplace_back(lower_left, uv.lower_left());
    vertices.emplace_back(lower_right, uv.lower_right());
    vertices.emplace_back(upper_left, uv.upper_left());
    vertices.emplace_back(lower_right, uv.lower_right());
    vertices.emplace_back(upper_right, uv.upper_right());
    vertices.emplace_back(upper_left, uv.upper_left());
  }
  group.count = dl.glyph_2d_vertices.size() - initial_vertex_count;
}

// Shape2d

void DrawBuilder::queue_box(
    const Box2& box,
    const Color& color,
    float border_width,
    Color border_color,
    float radius) {
  dl.shape_2d_instances.push_back(
      Shape2dInstance::box(box, color, border_width, border_color, radius));
  queue_shape_2d();
}

void DrawBuilder::queue_rect(
    const Vec2& position,
    float angle,
    const Vec2& size,
    const Color& color,
    float border_width,
    Color border_color) {
  dl.shape_2d_instances.push_back(
      Shape2dInstance::rect(
          position,
          angle,
          size,
          color,
          border_width,
          border_color));
  queue_shape_2d();
}

void DrawBuilder::queue_circle(
    const Vec2& position,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  dl.shape_2d_instances.push_back(
      Shape2dInstance::circle(
          position,
          radius,
          color,
          border_width,
          border_color));
  queue_shape_2d();
}

void DrawBuilder::queue_ellipse(
    const Vec2& position,
    float angle,
    const Vec2& radii,
    const Color& color,
    float border_width,
    Color border_color) {
  dl.shape_2d_instances.push_back(
      Shape2dInstance::ellipse(
          position,
          angle,
          radii,
          color,
          border_width,
          border_color));
  queue_shape_2d();
}

void DrawBuilder::queue_line(
    const Vec2& a,
    const Vec2& b,
    float width,
    const Color& color,
    bool rounded_ends) {
  dl.shape_2d_instances.push_back(
      Shape2dInstance::line(a, b, width, color, rounded_ends));
  queue_shape_2d();
}

void DrawBuilder::queue_capsule(
    const Vec2& start,
    const Vec2& end,
    float radius,
    const Color& color,
    float border_width,
    Color border_color) {
  dl.shape_2d_instances.push_back(
      Shape2dInstance::capsule(
          start,
          end,
          radius,
          color,
          border_width,
          border_color));
  queue_shape_2d();
}

} // namespace dgui
