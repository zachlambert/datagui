#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct TextInputStyle {
  BoxStyle box;
  TextStyle text;
  Color focus_color;

  void apply(const StyleManager& style) {
    box.apply(style);
    text.apply(style);
    style.focus_color(focus_color);
  }
};

struct TextInputData {
  TextInputStyle style;
  std::string text;
  bool changed = false;
};

class TextInputSystem : public ElementSystemImpl<TextInputData> {
public:
  TextInputSystem(Resources& res) : res(res) {}

  const std::string* visit(Element element, const std::string& initial_text);
  void visit(Element element, const Variable<std::string>& text);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

  void mouse_event(Element element, const MouseEvent& event) override;
  void key_event(Element element, const KeyEvent& event) override;
  void text_event(Element element, const TextEvent& event) override;

  void focus_enter(Element element) override;
  void focus_leave(Element element, bool success, ConstElement new_element)
      override;

private:
  Resources& res;

  std::string active_text;
  TextSelection active_selection;
};

} // namespace datagui
