#pragma once

#include "datagui/tree/element.hpp"

namespace datagui {

struct SeriesProps {
  // Style
  Direction direction = Direction::Vertical;
  Alignment alignment = Alignment::Center;
  Length length = LengthWrap();
  Length width = LengthDynamic();
  Color bg_color = Color::Clear();
  BoxDims outer_padding = 5;
  float inner_padding = 5;

  // State
  float overrun = 0;
  float scroll_pos = 0;

  void set_style(const StyleManager& sm) {
    sm.series_direction(direction);
    sm.series_alignment(alignment);
    sm.series_length(length);
    sm.series_width(width);
    sm.series_bg_color(bg_color);
    sm.series_outer_padding(outer_padding);
    sm.series_inner_padding(inner_padding);
  }
};

struct SeriesSystem : public ElementSystem {
  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(const Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) const override;
  bool scroll_event(Element& element, const ScrollEvent& event) override;
};

} // namespace datagui
