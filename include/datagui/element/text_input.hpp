#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct TextInputStyle {
  Length width = LengthDynamic(1);
  BoxDims padding;
  Color bg_color;
  BoxDims border_width;
  Color border_color;
  TextStyle text;
  InputStyle input;

  void apply(const StyleManager& style) {
    style.text_input_width(width);
    style.text_padding(padding);
    style.text_input_bg_color(bg_color);
    style.text_input_border_width(border_width);
    style.text_input_border_color(border_color);
    text.apply(style);
    input.apply(style);
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
