#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"
#include <optional>

namespace datagui {

class FloatingSystem : public ElementSystem {
public:
  FloatingSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;
  bool mouse_event(Element& element, const MouseEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

inline FloatingProps& get_floating(
    ElementSystemList& systems,
    Element& element,
    bool initial_open_value) {
  if (element.system == -1) {
    element.system = systems.find<FloatingSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<FloatingProps>();
    auto& props = *element.props.cast<FloatingProps>();
    props.open = initial_open_value;
  }
  auto props = element.props.cast<FloatingProps>();
  assert(props);
  return *props;
}

} // namespace datagui
