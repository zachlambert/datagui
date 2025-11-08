#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"

namespace datagui {

struct TextInputProps {
  std::string text;
  bool changed = false;
  Length width = LengthDynamic(1);
};

class TextInputSystem : public ElementSystem {
public:
  TextInputSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;

  bool mouse_event(Element& element, const MouseEvent& event) override;
  bool key_event(Element& element, const KeyEvent& event) override;
  bool text_event(Element& element, const TextEvent& event) override;

  void focus_enter(Element& element) override;
  bool focus_leave(Element& element, bool success) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
  std::string active_text;
  TextSelection active_selection;
};

inline TextInputProps& get_text_input(
    ElementSystemList& systems,
    Element& element,
    const std::string& initial_value) {
  if (element.system == -1) {
    element.system = systems.find<TextInputSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<TextInputProps>();
    auto& props = *element.props.cast<TextInputProps>();
    props.text = initial_value;
  }
  auto props = element.props.cast<TextInputProps>();
  assert(props);
  return *props;
}

} // namespace datagui
