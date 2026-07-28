#pragma once

#include "datagui/color.hpp"
#include "datagui/render/draw_list_old.hpp"
#include "datagui/render/font_atlas.hpp"
#include <optional>

namespace dgui {

class DrawBuilder {
public:
  void begin_group(int z_index, const Box2& mask);
  void begin_scene_2d(int z_index, const Color& bg_color, const Box2& mask);
  void queue_scene_3d(
      int z_index,
      const Color& bg_color,
      const Box2& mask,
      Scene3d* scene_3d);
  DrawList flush();

  void queue_text(
      const FontAtlas& font_atlas,
      const Vec2& origin,
      double angle,
      const Vec2& scale,
      const Color& color,
      Length width,
      const std::string& text);

  void queue_image(
      const Image& image,
      const Vec2& position,
      float angle,
      const Vec2& size);

  void queue_box(
      const Box2& box,
      const Color& color,
      float border_width,
      Color border_color,
      float radius);

  void queue_rect(
      const Vec2& position,
      float angle,
      const Vec2& size,
      const Color& color,
      float border_width,
      Color border_color);

  void queue_circle(
      const Vec2& position,
      float radius,
      const Color& color,
      float border_width,
      Color border_color);

  void queue_ellipse(
      const Vec2& position,
      float angle,
      const Vec2& radii,
      const Color& color,
      float border_width,
      Color border_color);

  void queue_line(
      const Vec2& a,
      const Vec2& b,
      float width,
      const Color& color,
      bool rounded_ends);

  void queue_capsule(
      const Vec2& start,
      const Vec2& end,
      float radius,
      const Color& color,
      float border_width,
      Color border_color);

private:
  void add_batch(int z_index, const Box2& mask);
  void queue_shape_2d();

  DrawList dl;
  int batch_z_index = 0;
  DrawList::Batch* batch = nullptr;

  enum class CallType { Geometry, Image, Text };
  std::optional<CallType> prev_call_type_;
  bool in_scene_2d_ = false;
};

} // namespace dgui
