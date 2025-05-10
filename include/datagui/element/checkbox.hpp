#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct CheckboxStyle {
  float size;
  BoxStyle box;
  Color icon_color = Color::Black();
  BoxDims inner_padding = 0;

  void apply(const StyleManager& style) {
    style.checkbox_size(size);
    box.width = LengthFixed(size);
    box.height = LengthFixed(size);
    box.padding = BoxDims(0);
    style.checkbox_bg_color(box.bg_color);
    style.checkbox_border_color(box.border_color);
    style.checkbox_border_width(box.border_width);
    style.checkbox_radius(box.radius);
    style.checkbox_icon_color(icon_color);
    style.checkbox_inner_padding(inner_padding);
  }
};

struct CheckboxData {
  CheckboxStyle style;
  bool checked = false;
  bool changed = false;
};

class CheckboxSystem : public ElementSystemImpl<CheckboxData> {
public:
  CheckboxSystem(Resources& res) : res(res) {}

  const bool* visit(Element element, const bool& initial_checked);
  void visit(Element element, const Variable<bool>& checked);

  void set_layout_input(Element element) const override;
  void render(ConstElement element) const override;

  void mouse_event(Element element, const MouseEvent& event) override;
  void key_event(Element element, const KeyEvent& event) override;

private:
  Resources& res;

  std::string active_text;
  TextSelection active_selection;
};

} // namespace datagui
