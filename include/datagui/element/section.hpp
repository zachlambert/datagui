#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

struct SectionProps {
  std::string label;
  bool open = false;
};

class SectionSystem : public ElementSystem {
public:
  SectionSystem(std::shared_ptr<FontManager> fm, std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

  bool mouse_event(Element& element, const MouseEvent& event) override;
  bool key_event(Element& element, const KeyEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

inline SectionProps& get_section(
    ElementSystemList& systems,
    Element& element,
    const std::string& label) {
  if (element.system == -1) {
    element.system = systems.find<SectionSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<SectionProps>();
    auto& props = *element.props.cast<SectionProps>();
    props.open = false;
  }
  auto props = element.props.cast<SectionProps>();
  assert(props);
  props->label = label;
  return *props;
}

} // namespace datagui
