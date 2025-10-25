#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"
#include <assert.h>

namespace datagui {

struct ButtonProps {
  // Data
  std::string text;
  bool released = false;
  bool down = false;
  Length width = LengthWrap();
};

class ButtonSystem : public ElementSystem {
public:
  ButtonSystem(std::shared_ptr<FontManager> fm, std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  bool visit(Element element, const std::string& text);

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

  bool mouse_event(Element& element, const MouseEvent& event) override;
  bool key_event(Element& element, const KeyEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
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
