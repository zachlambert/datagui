#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

struct CheckboxProps {
  // Style
  float size = 24;
  Color bg_color = Color::White();
  BoxDims border_width = 2;
  Color border_color = Color::Black();
  float radius = 0;
  Color icon_color = Color::Black();
  BoxDims inner_padding = 2;

  // Data
  bool checked = false;
  bool changed = false;

  void set_style(const StyleManager& sm) {
    sm.checkbox_size(size);
    sm.checkbox_bg_color(bg_color);
    sm.checkbox_border_width(border_width);
    sm.checkbox_border_color(border_color);
    sm.checkbox_radius(radius);
    sm.checkbox_icon_color(icon_color);
    sm.checkbox_inner_padding(inner_padding);
  }
};

class CheckboxSystem : public ElementSystem {
public:
  CheckboxSystem(std::shared_ptr<FontManager> fm) : fm(fm) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

  bool mouse_event(Element& element, const MouseEvent& event) override;
  bool key_event(Element& element, const KeyEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
};

inline CheckboxProps& get_checkbox(
    ElementSystemList& systems,
    Element& element,
    bool initial_value) {
  if (element.system == -1) {
    element.system = systems.find<CheckboxSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<CheckboxProps>();
    auto props = *element.props.cast<CheckboxProps>();
    props.checked = initial_value;
  }
  auto props = element.props.cast<CheckboxProps>();
  assert(props);
  return *props;
}

} // namespace datagui
