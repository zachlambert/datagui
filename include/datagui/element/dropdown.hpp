#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct DropdownStyle {
  Length width = LengthWrap();
  Color bg_color = Color::Gray(0.8);
  BoxDims border_width = 2;
  Color border_color = Color::Black();
  BoxDims padding = 5;
  float inner_border_width = 2;
  Color inner_border_color = Color::Black();
  TextStyle text;
  InputStyle input;

  void apply(const StyleManager& style) {
    style.dropdown_width(width);
    style.dropdown_bg_color(bg_color);
    style.dropdown_border_width(border_width);
    style.dropdown_border_color(border_color);
    style.text_padding(padding);
    style.dropdown_inner_border_width(inner_border_width);
    style.dropdown_inner_border_color(inner_border_color);
    text.apply(style);
    input.apply(style);
  }
};

struct DropdownData {
  DropdownStyle style;
  std::vector<std::string> choices;
  int choice = -1;
  int choice_hovered = -1;
  bool changed = false;
  bool open = false;
};

class DropdownSystem : public ElementSystemImpl<DropdownData> {
public:
  DropdownSystem(Resources& res) : res(res) {}

  const int* visit(
      Element element,
      const std::vector<std::string>& choices,
      int initial_choice);
  void visit(
      Element element,
      const std::vector<std::string>& choices,
      const Variable<int>& choice);

  void set_input_state(Element element) const override;
  void set_dependent_state(Element element) const override;
  void render(ConstElement element) const override;
  void mouse_event(Element element, const MouseEvent& event) override;

  void focus_enter(Element element) override;
  void focus_leave(Element element, bool success, ConstElement new_element)
      override;

private:
  Resources& res;
};

} // namespace datagui
