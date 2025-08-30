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

class TextBoxSystem : public ElementSystem {
public:
  TextBoxSystem(std::shared_ptr<FontManager> fm) : fm(fm) {}
  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) const override;

private:
  std::shared_ptr<FontManager> fm;
};

} // namespace datagui
