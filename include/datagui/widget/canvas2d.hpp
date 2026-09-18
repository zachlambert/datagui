#pragma once

#include "datagui/widget/widget.hpp"
#include "datagui/render/state/scene_2d.hpp"
#include "datagui/render/font_registry.hpp"
#include <functional>
#include <optional>
#include <string>
#include <vector>

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

  // Labels are always drawn on top of the scene geometry, regardless of the
  // order they are added in
  void label(
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

  // View the camera is reset to, on the first update and whenever the view is
  // reset by double clicking
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
  void end() override;

  Vec2 min_size() const override { return Vec2::uniform(border_width_ * 2); }
  void set_dependent_state(const Box2& box) override;
  void render(const Box2& box, DrawList& dl) const override;

  void mouse_event(const Box2& box, const MouseEvent& event) override;
  bool scroll_event(const Box2& box, const ScrollEvent& event) override;
  void reset_camera();

  // Labels are queued instead of being written to the scene immediately, since
  // the scale depends on the widget size, which isn't known until
  // set_dependent_state()
  struct LabelCommand {
    std::string text;
    Vec2 origin;
    float angle;
    int font_size;
    Font font;
    Color text_color;
    Length width;
  };

  static constexpr float border_width_ = 2;

  bool first_visit_ = true;
  Camera2d click_camera;
  std::optional<MouseEvent> mouse_event_;

  Vec2 default_position_;
  float default_view_width_ = 1;

  float zoom = 1;
  std::vector<LabelCommand> label_commands_;

  std::shared_ptr<FontRegistry> font_registry;
  Camera2d camera;
  std::shared_ptr<Scene2d> scene;
};

} // namespace dgui
