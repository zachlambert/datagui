#pragma once

#include "datagui/theme.hpp"
#include "datagui/tree/element.hpp"
#include <optional>

namespace datagui {

struct SeriesProps {
  float overrun = 0;
  float scroll_pos = 0;

  Direction direction = Direction::Vertical;
  Alignment alignment = Alignment::Center;
  Length length = LengthWrap();
  Length width = LengthDynamic(1);
  bool no_padding = false;
  std::optional<Color> bg_color;
};

class SeriesSystem : public ElementSystem {
public:
  SeriesSystem(std::shared_ptr<Theme> theme) : theme(theme) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;
  bool scroll_event(Element& element, const ScrollEvent& event) override;

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
