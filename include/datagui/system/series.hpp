#pragma once

#include "datagui/system/system.hpp"
#include "datagui/theme.hpp"
#include <optional>

namespace datagui {

class SeriesSystem : public System {
public:
  SeriesSystem(std::shared_ptr<Theme> theme) : theme(theme) {}

  void set_input_state(ElementPtr element) override;
  void set_dependent_state(ElementPtr element) override;
  void render(ConstElementPtr element, Renderer& renderer) override;
  bool scroll_event(ElementPtr element, const ScrollEvent& event) override;

private:
  std::shared_ptr<Theme> theme;
};

inline SeriesProps& get_series(ElementSystemList& systems, Element& element) {
  if (element.system == -1) {
    element.system = systems.find<SeriesSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<SeriesProps>();
  }
  auto props = element.props.cast<SeriesProps>();
  assert(props);
  return *props;
}

} // namespace datagui
