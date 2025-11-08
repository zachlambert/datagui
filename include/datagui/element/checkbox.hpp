#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

struct CheckboxProps {
  bool checked = false;
  bool changed = false;
};

class CheckboxSystem : public ElementSystem {
public:
  CheckboxSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

  bool mouse_event(Element& element, const MouseEvent& event) override;
  bool key_event(Element& element, const KeyEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
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
    auto& props = *element.props.cast<CheckboxProps>();
    props.checked = initial_value;
  }
  auto props = element.props.cast<CheckboxProps>();
  assert(props);
  return *props;
}

} // namespace datagui
