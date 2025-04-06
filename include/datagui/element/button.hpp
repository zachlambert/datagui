#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct ButtonStyle : public BoxStyle, public TextStyle {
  Color focus_color = Color(0, 1, 1);
  Color down_color = Color::Gray(0.6);
  ButtonStyle() {
    bg_color = Color::Gray(0.8);
    border_width = 2;
    border_color = Color::Gray(0.5);
    padding = 10;
  }
};

struct ButtonElement {
  using Style = ButtonStyle;
  std::string text;
  bool released = false;
  bool down = false;
  Style style;
};

class ButtonSystem : public ElementSystemBase<ButtonElement> {
public:
  ButtonSystem(
      FontManager& font_manager,
      GeometryRenderer& geometry_renderer,
      TextRenderer& text_renderer) :
      font_manager(font_manager),
      geometry_renderer(geometry_renderer),
      text_renderer(text_renderer) {}

  void init(const std::function<void(TextStyle&)> set_style);
  void set_layout_input(Tree::Ptr node) const override;
  void render(Tree::ConstPtr node) const override;

  void mouse_event(Tree::Ptr node, const MouseEvent& event) override;
  void key_event(Tree::Ptr node, const KeyEvent& event) override;

private:
  FontManager& font_manager;
  GeometryRenderer& geometry_renderer;
  TextRenderer& text_renderer;
};

} // namespace datagui
