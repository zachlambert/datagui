#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

struct LabelledProps {
  std::string label;
};

class LabelledSystem : public ElementSystem {
public:
  LabelledSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

inline LabelledProps& get_labelled(
    ElementSystemList& systems,
    Element& element,
    const std::string& label) {
  if (element.system == -1) {
    element.system = systems.find<LabelledSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<LabelledProps>();
    auto& props = *element.props.cast<LabelledProps>();
  }
  auto props = element.props.cast<LabelledProps>();
  assert(props);
  props->label = label;
  return *props;
}

} // namespace datagui
