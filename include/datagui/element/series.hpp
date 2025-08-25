#pragma once

#include "datagui/tree/element.hpp"

namespace datagui {

struct SeriesStyle {
  Direction direction = Direction::Vertical;
  Alignment alignment = Alignment::Center;
  Length length = LengthWrap();
  Length width = LengthDynamic();
  Color bg_color = Color::Clear();
  BoxDims outer_padding = 5;
  float inner_padding = 5;

  void apply(const StyleManager& style) {
    style.series_direction(direction);
    style.series_alignment(alignment);
    style.series_length(length);
    style.series_width(width);
    style.series_bg_color(bg_color);
    style.series_outer_padding(outer_padding);
    style.series_inner_padding(inner_padding);
  }
};

struct SeriesElement : public Element {
  SeriesStyle style;
  float overrun = 0;
  float scroll_pos = 0;

  SeriesElement(Resources* resources) : Element(resources) {}
  void set_input_state(const std::vector<const Element*>& children) override;
  void set_dependent_state(const std::vector<Element*>& children) override;
  void render() const override;
  bool scroll_event(const ScrollEvent& event) override;
};

} // namespace datagui
