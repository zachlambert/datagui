#pragma once

#include "datagui/tree/element.hpp"

namespace datagui {

struct TextBoxStyle {
  BoxDims padding = 5;
  TextStyle text;

  void apply(const StyleManager& style) {
    style.text_padding(padding);
    text.apply(style);
  }
};

struct TextBoxElement : public Element {
  TextBoxStyle style;
  std::string text;

  TextBoxElement(Resources* resources) : Element(resources) {}
  void set_input_state(const std::vector<const Element*>& children) override;
  void render() const override;
};

} // namespace datagui
