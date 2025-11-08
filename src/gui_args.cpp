#include "datagui/gui_args.hpp"

#include "datagui/element/floating.hpp"
#include "datagui/element/series.hpp"

namespace datagui {

void SeriesArgs::apply(SeriesProps& props) const {
  props.direction = direction_;
  props.alignment = alignment_;
  props.length = length_;
  props.width = width_;
  props.no_padding = no_padding_;
  props.bg_color = bg_color_;
}

void FloatingArgs::apply(FloatingProps& props) const {
  props.bg_color = bg_color_;
}

} // namespace datagui
