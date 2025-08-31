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
  void render(const Element& element, Renderer& renderer) override;

private:
  std::shared_ptr<FontManager> fm;
};

inline TextBoxProps& get_text_box(
    ElementSystemList& systems,
    Element& element) {
  if (element.system == -1) {
    element.system = systems.find<TextBoxSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<TextBoxProps>();
  }
  auto props = element.props.cast<TextBoxProps>();
  assert(props);
  return *props;
}

} // namespace datagui
