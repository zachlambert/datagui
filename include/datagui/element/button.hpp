#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct ButtonStyle : public BoxStyle, public TextStyle {
  Color focus_color = Color(0, 1, 1);
  Color down_color = Color::Gray(0.4);
  Color hover_color = Color::Gray(0.6);
  Length width = literals::_wrap;
  ButtonStyle() {
    bg_color = Color::Gray(0.8);
    border_width = 2;
    border_color = Color::Gray(0.5);
    padding = 10;
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
