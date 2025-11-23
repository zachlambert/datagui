#pragma once

#include "datagui/input/text_selection.hpp"
#include "datagui/system/system.hpp"
#include "datagui/theme.hpp"

namespace datagui {

class TextInputSystem : public System {
public:
  TextInputSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;

  bool mouse_event(ElementPtr element, const MouseEvent& event) override;
  bool key_event(ElementPtr element, const KeyEvent& event) override;
  bool text_event(ElementPtr element, const TextEvent& event) override;

  void focus_enter(ElementPtr element) override;
  bool focus_leave(ElementPtr element, bool success) override;

private:
  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
  std::string active_text;
  TextSelection active_selection;
};

} // namespace datagui
