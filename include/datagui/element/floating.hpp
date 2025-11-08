#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"

namespace datagui {

struct FloatingProps {
  std::string title;
  bool open = false;
  bool open_changed = false;
  Boxf title_bar_box;
  float title_bar_text_width;
  Boxf close_button_box;
  FloatingType float_type = FloatingTypeRelative(Vecf::Zero(), Vecf::Zero());
};

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
