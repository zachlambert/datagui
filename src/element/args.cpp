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

void SeriesArgs::reset() {
  direction_ = Direction::Vertical;
  alignment_ = Alignment::Center;
  length_ = LengthWrap();
  width_ = LengthDynamic(1);
  no_padding_ = false;
  bg_color_ = std::nullopt;
}

void FloatingArgs::apply(Floating& props) const {
  props.bg_color = bg_color_;
}

void FloatingArgs::reset() {
  bg_color_ = std::nullopt;
}

} // namespace datagui
