#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/font_manager.hpp"

namespace datagui {

struct DropdownProps {
  std::vector<std::string> choices;
  int choice = -1;
  int choice_hovered = -1;
  bool changed = false;
  bool open = false;
  Length width = LengthWrap();
};

class DropdownSystem : public ElementSystem {
public:
  DropdownSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;
  bool mouse_event(Element& element, const MouseEvent& event) override;

  void focus_enter(Element& element) override;
  bool focus_leave(Element& element, bool success) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

inline DropdownProps& get_dropdown(
    ElementSystemList& systems,
    Element& element,
    const std::vector<std::string>& choices,
    int initial_choice) {
  if (element.system == -1) {
    element.system = systems.find<DropdownSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<DropdownProps>();
    auto& props = *element.props.cast<DropdownProps>();
    props.choices = choices;
    props.choice = initial_choice;
  }
  auto props = element.props.cast<DropdownProps>();
  assert(props);
  return *props;
}

} // namespace datagui
