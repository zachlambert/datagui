#pragma once

#include "datagui/asset/image.hpp"
#include "datagui/geometry/camera.hpp"
#include "datagui/render/state/glyph_2d_instance.hpp"
#include "datagui/render/state/scene_2d.hpp"
#include "datagui/render/state/scene_3d.hpp"
#include "datagui/render/state/shape_2d_instance.hpp"

namespace dgui {

struct DrawList {
  struct GlyphGroup {
    unsigned int font_texture;
    size_t offset;
    size_t count;
  };
  struct ImageInstance {
    Image image;
    Mat3 transform;
  };
  struct Scene2dInstance {
    Box2 viewport;
    Camera2d camera;
    std::shared_ptr<Scene2d> scene_2d;
  };
  struct Scene3dInstance {
    Box2 viewport;
    Camera3d camera;
    std::shared_ptr<Scene3d> scene_3d;
  };

  std::vector<Shape2dInstance> shape_instances;
  std::vector<Glyph2dInstance> glyph_instances;
  std::vector<GlyphGroup> glyph_groups;
  std::vector<ImageInstance> image_instances;
  std::vector<Scene2dInstance> scene_2d_instances;
  std::vector<Scene3dInstance> scene_3d_instances;

  struct Group {
    Box2 mask;
    size_t shape_offset;
    size_t shape_count;
    size_t glyph_group_offset;
    size_t glyph_group_count;
    size_t image_offset;
    size_t image_count;
    size_t scene_2d_offset;
    size_t scene_2d_count;
    size_t scene_3d_offset;
    size_t scene_3d_count;
  };
  std::vector<Group> groups;

  void new_group(const Box2& mask);

  void draw_box(
      const Box2& box,
      const Color& color,
      float border_width = 0,
      Color border_color = Color::Black(),
      float radius = 0);

  void draw_text(
      const FontAtlas& font_atlas,
      const Vec2& origin,
      const Color& color,
      Length width,
      const std::string& text);

  void draw_image(
      const Image& image,
      const Vec2& origin,
      double angle,
      Vec2& scale);

  void draw_scene_2d(
      const Box2& viewport,
      const Camera2d& camera,
      const std::shared_ptr<Scene2d>& scene_2d);

  void draw_scene_3d(
      const Box2& viewport,
      const Camera3d& camera,
      const std::shared_ptr<Scene3d>& scene_3d);

  void clear();
};

} // namespace dgui
