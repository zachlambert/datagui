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
    const FontAtlas& font_atlas,
    const Vec2& origin,
    double angle,
    const Vec2& scale,
    const Color& color,
    Length width,
    const std::string& text) {
  if (in_scene_2d_ && (prev_call_type_ && *prev_call_type_ != CallType::Text)) {
    add_batch(batch_z_index, batch->mask);
  }
  prev_call_type_ = CallType::Text;

  if (batch->glyph_groups.empty() ||
      !batch->glyph_groups.back().matches(font_atlas.texture(), color)) {
    auto& group = batch->glyph_groups.emplace_back();
    group.font_texture = font_atlas.texture();
    group.color = color;
    group.offset = dl.glyph_2d_vertices.size();
  }
  auto& group = batch->glyph_groups.back();

  group.count +=
      font_atlas
          .add_glyphs(dl.glyph_2d_vertices, origin, angle, scale, text, width);
}

void DrawBuilder::queue_image(
    const Image& image,
    const Vec2& position,
    float angle,
    const Vec2& size) {
  if (!image.is_loaded()) {
    return;
  }
  if (in_scene_2d_ &&
      (prev_call_type_ && *prev_call_type_ != CallType::Image)) {
    add_batch(batch_z_index, batch->mask);
  }
  prev_call_type_ = CallType::Image;

  if (batch->image_groups.empty() ||
      !batch->image_groups.back().matches(image)) {
    auto& group = batch->image_groups.emplace_back();
    group.image = image;
    group.offset = dl.image_2d_vertices.size();
  }
  auto& group = batch->image_groups.back();

  Mat2 rot = Rot2(angle).mat();
  Vec2 lower_left = position;
  Vec2 lower_right = position + rot * Vec2(size.x, 0);
  Vec2 upper_left = position + rot * Vec2(0, size.y);
  Vec2 upper_right = position + rot * size;

  auto& vertices = dl.image_2d_vertices;
  const size_t initial_vertex_count = vertices.size();

  // UV V is flipped so the image's top row maps to the top of the quad.
  vertices.emplace_back(lower_left, Vec2(0, 1));
  vertices.emplace_back(lower_right, Vec2(1, 1));
  vertices.emplace_back(upper_left, Vec2(0, 0));
  vertices.emplace_back(lower_right, Vec2(1, 1));
  vertices.emplace_back(upper_right, Vec2(1, 0));
  vertices.emplace_back(upper_left, Vec2(0, 0));

  group.count = dl.image_2d_vertices.size() - initial_vertex_count;
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
