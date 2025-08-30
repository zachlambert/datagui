#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

struct DropdownProps {
  // Style
  Length width = LengthWrap();
  Color bg_color = Color::Gray(0.8);
  BoxDims border_width = 2;
  Color border_color = Color::Black();
  BoxDims padding = 5;
  float inner_border_width = 2;
  Color inner_border_color = Color::Black();
  TextStyle text_style;
  InputStyle input_style;

  // Data
  std::vector<std::string> choices;
  int choice = -1;
  int choice_hovered = -1;
  bool changed = false;
  bool open = false;

  void set_style(const StyleManager& sm) {
    sm.dropdown_width(width);
    sm.dropdown_bg_color(bg_color);
    sm.dropdown_border_width(border_width);
    sm.dropdown_border_color(border_color);
    sm.text_padding(padding);
    sm.dropdown_inner_border_width(inner_border_width);
    sm.dropdown_inner_border_color(inner_border_color);
    text_style.apply(sm);
    input_style.apply(sm);
  }
};

class DropdownSystem : public ElementSystem {
public:
  DropdownSystem(std::shared_ptr<FontManager> fm) : fm(fm) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;
  bool mouse_event(Element& element, const MouseEvent& event) override;

  void focus_enter(Element& element) override;
  bool focus_leave(Element& element, bool success) override;

private:
  std::shared_ptr<FontManager> fm;
};

} // namespace datagui
