#pragma once

#include "datagui/tree/element.hpp"

namespace datagui {

struct ButtonProps {
  // Style
  Length width = LengthWrap();
  BoxDims padding = 5;
  Color bg_color = Color::Gray(0.8);
  BoxDims border_width = 2;
  Color border_color = Color::Black();
  float radius = 0;
  TextStyle text_style;
  InputStyle input_style;

  // Data
  std::string text;
  bool released = false;
  bool down = false;

  void set_style(const StyleManager& sm) {
    sm.button_width(width);
    sm.text_padding(padding);
    sm.button_bg_color(bg_color);
    sm.button_border_width(border_width);
    sm.button_border_color(border_color);
    sm.button_radius(radius);
    text_style.apply(sm);
    input_style.apply(sm);
  }
};

class ButtonSystem : public ElementSystem {
public:
  ButtonSystem(std::shared_ptr<FontManager> fm) : fm(fm) {}

  bool visit(Element element, const std::string& text);

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

  bool mouse_event(Element& element, const MouseEvent& event) override;
  bool key_event(Element& element, const KeyEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
};

inline ButtonProps& get_button(ElementSystemList& systems, Element& element) {
  if (element.system == -1) {
    element.system = systems.find<ButtonSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<ButtonProps>();
  }
  auto props = element.props.cast<ButtonProps>();
  assert(props);
  return *props;
}

} // namespace datagui
