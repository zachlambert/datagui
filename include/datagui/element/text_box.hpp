#pragma once

#include "datagui/tree/element.hpp"

namespace datagui {

struct TextBoxProps {
  // Style
  BoxDims padding = 5;
  TextStyle text_style;

  // Data
  std::string text;

  void set_style(const StyleManager& sm) {
    sm.text_padding(padding);
    text_style.apply(sm);
  }
};

struct TextBoxSystem : public ElementSystem {
  TextBoxSystem(Resources* resources) : ElementSystem(resources) {}
  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element) const override;
};

} // namespace datagui
