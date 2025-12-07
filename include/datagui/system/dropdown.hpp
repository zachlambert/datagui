#pragma once

#include "datagui/element/system.hpp"
#include "datagui/theme.hpp"

namespace datagui {

class DropdownSystem : public System {
public:
  DropdownSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;
  bool scroll_event(ElementPtr element, const ScrollEvent& event) override;
  void focus_enter(ElementPtr element) override;
  void focus_tree_leave(ElementPtr element) override;

private:
  std::string get_slider_text(const Slider& slider) const;

  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
