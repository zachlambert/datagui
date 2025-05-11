#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

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

struct SeriesData {
  SeriesStyle style;
  float overrun = 0;
  float scroll_pos = 0;
};

class SeriesSystem : public ElementSystemImpl<SeriesData> {
public:
  SeriesSystem(Resources& res) : res(res) {}

  void visit(Element element);

  void set_layout_input(Element element) const override;
  void set_child_layout_output(Element element) const override;
  void render(ConstElement element) const override;
  bool scroll_event(Element element, const ScrollEvent& event) override;

private:
  Resources& res;
};

} // namespace datagui
