#include "datagui/render/state/draw_list.hpp"

namespace dgui {

void DrawList::clear_mask() {}

void DrawList::new_group(const Box2& mask, bool floating) {
  Group group;
  group.mask = floating || groups.empty()
                   ? mask
                   : intersection(mask, groups.back().mask);
  group.shape_offset = shape_instances.size();
  group.glyph_group_offset = glyph_groups.size();
  group.image_offset = image_instances.size();
  group.scene_2d_offset = scene_2d_instances.size();
  group.scene_3d_offset = scene_3d_instances.size();
  groups.push_back(group);
}

void DrawList::draw_box(
    const Box2& box,
    const Color& color,
    float border_width,
    Color border_color,
    float radius) {
  if (groups.empty()) {
    throw std::logic_error("Must call new_group() first");
  }
  auto& group = groups.back();
  group.shape_count++;
  shape_instances.push_back(
      Shape2dInstance::box(box, color, border_width, border_color, radius));
}

void DrawList::draw_text(
    const FontAtlas& font_atlas,
    const Vec2& origin,
    const Color& color,
    Length width,
    const std::string& text,
    bool editable) {
  if (groups.empty()) {
    throw std::logic_error("Must call new_group() first");
  }
  auto& group = groups.back();
  GlyphGroup* glyph_group =
      group.glyph_group_count == 0
          ? nullptr
          : &glyph_groups
                [group.glyph_group_offset + group.glyph_group_count - 1];
  if (!glyph_group || glyph_group->font_texture != font_atlas.texture()) {
    glyph_group = &glyph_groups.emplace_back();
    glyph_group->font_texture = font_atlas.texture();
    glyph_group->offset = glyph_instances.size();
    group.glyph_group_count++;
  }
  glyph_group->count += font_atlas.add_glyphs(
      glyph_instances,
      origin,
      0,
      Vec2::ones(),
      true,
      color,
      text,
      width,
      editable);
}

void DrawList::draw_image(
    const Image& image,
    const Vec2& origin,
    double angle,
    const Vec2& scale) {
  if (groups.empty()) {
    throw std::logic_error("Must call new_group() first");
  }
  auto& group = groups.back();
  group.image_count++;
  image_instances.push_back(
      ImageInstance{image, Mat3::transform(origin, angle, scale)});
}

void DrawList::draw_scene_2d(
    const Box2& viewport,
    const Camera2d& camera,
    const std::shared_ptr<Scene2d>& scene_2d) {
  if (groups.empty()) {
    throw std::logic_error("Must call new_group() first");
  }
  auto& group = groups.back();
  group.scene_2d_count++;
  scene_2d_instances.push_back(Scene2dInstance{viewport, camera, scene_2d});
}

void DrawList::draw_scene_3d(
    const Box2& viewport,
    const Camera3d& camera,
    const std::shared_ptr<Scene3d>& scene_3d) {
  if (groups.empty()) {
    throw std::logic_error("Must call new_group() first");
  }
  auto& group = groups.back();
  group.scene_3d_count++;
  scene_3d_instances.push_back(Scene3dInstance{viewport, camera, scene_3d});
}

void DrawList::clear() {
  shape_instances.clear();
  glyph_instances.clear();
  glyph_groups.clear();
  image_instances.clear();
  scene_2d_instances.clear();
  scene_3d_instances.clear();
  groups.clear();
}

} // namespace dgui
