#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct ButtonStyle {
  BoxStyle box;
  TextStyle text;
  Color active_color = Color::Clear();
  Color hover_color = Color::Clear();
  Color focus_color = Color::Clear();

  void apply(const StyleManager& style) {
    box.apply(style);
    text.apply(style);
    style.active_color(active_color);
    style.hover_color(hover_color);
    style.focus_color(focus_color);
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
