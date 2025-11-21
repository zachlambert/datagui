#include "datagui/element/args.hpp"
#include "datagui/element/element.hpp"

namespace datagui {

void SeriesArgs::apply(Series& props) const {
  props.direction = direction_;
  props.alignment = alignment_;
  props.length = length_;
  props.width = width_;
  props.no_padding = no_padding_;
  props.bg_color = bg_color_;
}

void FloatingArgs::apply(Floating& props) const {
  props.bg_color = bg_color_;
}

} // namespace datagui
