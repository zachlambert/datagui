#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"

namespace datagui {

struct TextBoxProps {
  std::string text;
};

class TextBoxSystem : public ElementSystem {
public:
  TextBoxSystem(std::shared_ptr<FontManager> fm, std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}
  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
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
