#pragma once

#include "datagui/system/system.hpp"
#include "datagui/theme.hpp"

namespace datagui {

class ColorPickerSystem : public System {
public:
  ColorPickerSystem(
      std::shared_ptr<FontManager> fm,
      std::shared_ptr<Theme> theme);

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;

  bool mouse_event(ElementPtr element, const MouseEvent& event) override;
  bool focus_leave(ElementPtr element, bool success) override;

private:
  std::string get_slider_text(const Slider& slider) const;

  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
  Color active_value;
};

} // namespace datagui
