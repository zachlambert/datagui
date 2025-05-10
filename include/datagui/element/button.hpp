#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct ButtonStyle {
  BoxStyle box;
  TextStyle text;
  InputStyle input;

  void apply(const StyleManager& style) {
    style.button_width(box.width);
    box.height = LengthWrap();
    style.text_padding(box.padding);
    style.button_bg_color(box.bg_color);
    style.button_border_width(box.border_width);
    style.button_border_color(box.border_color);
    style.button_radius(box.radius);
    text.apply(style);
    input.apply(style);
  }

  Color active_color() const {
    return input.active_color(box.bg_color);
  }
  Color hover_color() const {
    return input.hover_color(box.bg_color);
  }
  Color focus_color() const {
    return input.focus_color(box.border_color);
  }
};

struct ButtonData {
  ButtonStyle style;
  std::string text;
  bool released = false;
  bool down = false;
};

class ButtonSystem : public ElementSystemImpl<ButtonData> {
public:
  ButtonSystem(Resources& res) : res(res) {}

  bool visit(Element element, const std::string& text);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

  void mouse_event(Element element, const MouseEvent& event) override;
  void key_event(Element element, const KeyEvent& event) override;

private:
  Resources& res;
};

} // namespace datagui
