#pragma once

#include "datagui/widget/widget.hpp"
#include "datagui/render/state/scene_2d.hpp"
#include "datagui/render/font_registry.hpp"
#include <functional>
#include <optional>

namespace dgui {

class Canvas2d : public Widget {
public:
  void rect(
      const Vec2& position,
      float angle,
      const Vec2& size,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void circle(
      const Vec2& position,
      float radius,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void ellipse(
      const Vec2& position,
      float angle,
      const Vec2& radii,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void line(const Vec2& a, const Vec2& b, float width, const Color& color);

  void capsule(
      const Vec2& a,
      const Vec2& b,
      float radius,
      const Color& color,
      float border_width = 0,
      const Color& border_color = Color::Black());

  void text(
      const std::string& text,
      const Vec2& origin,
      float angle = 0,
      int font_size = 20,
      Font font = Font::DejaVuSans,
      Color text_color = Color::Black(),
      Length width = LengthWrap());

  void heatmap(
      const Vec2& lower,
      const Vec2& upper,
      const std::function<float(float x, float y)>& function,
      float min_value,
      float max_value,
      std::size_t width = 256,
      std::size_t height = 256);

  void default_view_position(const Vec2& position) {
    default_position_ = position;
  }
  void default_view_width(float view_width) {
    default_view_width_ = view_width;
  }

  void bg_color(const Color& color) {
    scene->bg_color = color;
  }

  std::optional<MouseEvent> mouse_event();

private:
  void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontRegistry>& font_registry) override;
  void begin() override;

  Vec2 min_size() const override { return Vec2::uniform(border_width_ * 2); }
  void set_dependent_state(const Box2& box) override;
  void render(const Box2& box, DrawList& dl) const override;

  void mouse_event(const Box2& box, const MouseEvent& event) override;
  bool scroll_event(const Box2& box, const ScrollEvent& event) override;

  static constexpr float border_width_ = 2;
  Color bg_color_ = Color::Gray(0.95);
  Camera2d click_camera;
  std::optional<MouseEvent> mouse_event_;

  Vec2 default_position_;
  float default_view_width_;

  float zoom = 1;
  float view_width_ = 0;

  std::shared_ptr<FontRegistry> font_registry;
  Camera2d camera;
  std::shared_ptr<Scene2d> scene;
};

} // namespace dgui
