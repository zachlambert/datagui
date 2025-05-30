#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct ButtonStyle {
  Length width = LengthWrap();
  BoxDims padding = 5;
  Color bg_color = Color::Gray(0.8);
  BoxDims border_width = 2;
  Color border_color = Color::Black();
  float radius = 0;
  TextStyle text;
  InputStyle input;

  void apply(const StyleManager& style) {
    style.button_width(width);
    style.text_padding(padding);
    style.button_bg_color(bg_color);
    style.button_border_width(border_width);
    style.button_border_color(border_color);
    style.button_radius(radius);
    text.apply(style);
    input.apply(style);
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

  void set_input_state(Element element) const override;
  void render(ConstElement element) const override;

  void mouse_event(Element element, const MouseEvent& event) override;
  void key_event(Element element, const KeyEvent& event) override;

private:
  Resources& res;
};

} // namespace datagui
