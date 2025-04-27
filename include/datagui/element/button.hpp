#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"
#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

struct ButtonStyle : public BoxStyle, public TextStyle {
  Color focus_color = Color(0, 1, 1);
  Color down_color = Color::Gray(0.6);
  Length width = literals::_wrap;
  ButtonStyle() {
    bg_color = Color::Gray(0.8);
    border_width = 2;
    border_color = Color::Gray(0.5);
    padding = 10;
  }
};
using SetButtonStyle = SetStyle<ButtonStyle>;

struct ButtonData {
  using Style = ButtonStyle;
  std::string text;
  bool released = false;
  bool down = false;
  Style style;
};

class ButtonSystem : public ElementSystemImpl<ButtonData> {
public:
  ButtonSystem(
      FontManager& font_manager,
      GeometryRenderer& geometry_renderer,
      TextRenderer& text_renderer) :
      font_manager(font_manager),
      geometry_renderer(geometry_renderer),
      text_renderer(text_renderer) {}

  bool visit(
      Element element,
      const std::string& text,
      const SetButtonStyle& set_style);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

  void mouse_event(Element element, const MouseEvent& event) override;
  void key_event(Element element, const KeyEvent& event) override;

private:
  FontManager& font_manager;
  GeometryRenderer& geometry_renderer;
  TextRenderer& text_renderer;
};

} // namespace datagui
