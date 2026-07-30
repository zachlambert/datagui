#pragma once

#include "datagui/element/system.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/theme.hpp"

namespace dgui {

class SliderSystem : public System {
public:
  SliderSystem(
      std::shared_ptr<FontRegistry> font_registry,
      std::shared_ptr<Theme> theme) :
      font_registry(font_registry), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, DrawList& dl) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;

private:
  std::string get_slider_text(const Slider& slider) const;

  std::shared_ptr<FontRegistry> font_registry;
  std::shared_ptr<Theme> theme;
  double active_value;
};

} // namespace dgui
