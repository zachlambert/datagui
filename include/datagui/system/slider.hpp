#pragma once

#include "datagui/element/system.hpp"
#include "datagui/theme.hpp"

namespace datagui {

class SliderSystem : public System {
public:
  SliderSystem(std::shared_ptr<FontManager> fm, std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  void mouse_event(ElementPtr element, const MouseEvent& event) override;

private:
  std::string get_slider_text(const Slider& slider) const;

  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
  double active_value;
};

} // namespace datagui
