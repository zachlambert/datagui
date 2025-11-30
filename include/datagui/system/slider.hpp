#pragma once

#include "datagui/system/system.hpp"
#include "datagui/theme.hpp"

namespace datagui {

class SliderSystem : public System {
public:
  SliderSystem(std::shared_ptr<FontManager> fm, std::shared_ptr<Theme> theme) :
      fm(fm), theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  bool mouse_event(ElementPtr element, const MouseEvent& event) override;

private:
  Box2 get_slider_box(const State& state, const Slider& slider) const;
  std::string get_slider_text(const Slider& slider) const;

  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
};

} // namespace datagui
