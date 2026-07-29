#pragma once

#include "datagui/asset/image.hpp"
#include "datagui/asset/mesh.hpp"
#include "datagui/asset/point_cloud.hpp"
#include "datagui/render/draw_list.hpp"
#include "datagui/render/font_atlas.hpp"

namespace dgui {

struct DrawQueue {
public:
  void begin_group(const Box2& mask);
  void begin_scene_2d(const Box2& viewport, const Camera2d& camera);
  void begin_scene_3d(const Box2& viewport, const Camera3d& camera);

  // ========================================================================
  // Draw 2D shapes

  void draw_box(
      const Box2& box,
      const Color& color,
      float border_width,
      Color border_color,
      float radius);

  void draw_rect(
      const Vec2& position,
      float angle,
      const Vec2& size,
      const Color& color,
      float border_width,
      Color border_color);

  void draw_circle(
      const Vec2& position,
      float radius,
      const Color& color,
      float border_width,
      Color border_color);

  void draw_ellipse(
      const Vec2& position,
      float angle,
      const Vec2& radii,
      const Color& color,
      float border_width,
      Color border_color);

  void draw_line(
      const Vec2& a,
      const Vec2& b,
      float width,
      const Color& color,
      bool rounded_ends);

  void draw_capsule(
      const Vec2& start,
      const Vec2& end,
      float radius,
      const Color& color,
      float border_width,
      Color border_color);

  // ========================================================================
  // Draw 3D shapes

  void draw_box(
      const Vec3& position,
      const Rot3& orientation,
      const Vec3& scale,
      const Color& color);

  void draw_cylinder(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void draw_sphere(const Vec3& position, float radius, const Color& color);

  void draw_half_sphere(
      const Vec3& position,
      const Vec3& direction,
      float radius,
      const Color& color);

  void draw_cone(
      const Vec3& base_position,
      const Vec3& direction,
      float radius,
      float length,
      const Color& color);

  void draw_plane(
      const Vec3& position,
      const Rot3& orientation,
      const Vec2& scale,
      const Color& color);

  void draw_capsule(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color);

  void draw_arrow(
      const Vec3& start,
      const Vec3& end,
      float radius,
      const Color& color,
      float head_length_scale = 2,
      float head_radius_scale = 2);

  // ========================================================================
  // Draw other 2D objects

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

  // ========================================================================
  // Draw other 3D objects

  void draw_mesh(
    const Mesh& mesh,
    const Vec3& position,
    const Rot3& rotation,
    const Vec3& scale,
    const Color& base_color = Color::White());

  void draw_point_cloud(
    const PointCloud& point_cloud,
    const Vec3& position,
    const Rot3& rotation,
    const Vec3& scale,
    float point_size,
    const Color& base_color = Color::White());

  DrawList flush();

private:
  void draw_shape_2d(const Shape2dInstance& instance);
  void draw_shape_3d(Shape3dType, const Shape3dInstance& instance);

  void clear_active() {
    index = Index();
  }
  DrawList::DrawShape2d& draw_shape_2d() {
    return std::get<DrawList::DrawShape2d>(dl.actions[index.draw_shape_2d]);
  }
  DrawList::DrawGlyph2d& draw_glyph_2d() {
    return std::get<DrawList::DrawGlyph2d>(dl.actions[index.draw_glyph_2d]);
  }
  DrawList::DrawShape3d& draw_shape_3d(Shape3dType type) {
    return std::get<DrawList::DrawShape3d>(
        dl.actions[index.draw_shape_3d[int(type)]]);
  }

  DrawList dl;

  enum class Context {
    None, Group, Scene2d, Scene3d
  };
  Context context = Context::None;

  struct Index {
    int draw_shape_2d = -1;
    int draw_glyph_2d = -1;
    std::array<int, Shape3dTypeCount> draw_shape_3d = {-1};
  };
  Index index;
  // bool keep_order = false;
  // bool y_flipped = true;
};

} // namespace dgui
