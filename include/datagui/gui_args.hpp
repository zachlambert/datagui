#pragma once

#include "datagui/layout.hpp"

namespace datagui {

class SeriesProps;

class SeriesArgs {
public:
  SeriesArgs() {
    reset();
  }

  void horizontal() {
    direction_ = Direction::Horizontal;
  }
  void align_min() {
    alignment_ = Alignment::Min;
  }
  void align_max() {
    alignment_ = Alignment::Max;
  }

  void length_fixed(float size) {
    length_ = LengthFixed(size);
  }
  void length_dynamic(float weight) {
    length_ = LengthDynamic(weight);
  }

  void width_fixed(float size) {
    width_ = LengthFixed(size);
  }
  void width_wrap() {
    width_ = LengthWrap();
  }

  void no_padding() {
    no_padding_ = true;
  }

private:
  void apply(SeriesProps& props) const;

  void reset() {
    direction_ = Direction::Vertical;
    alignment_ = Alignment::Center;
    length_ = LengthWrap();
    width_ = LengthDynamic(1);
    no_padding_ = false;
  }

  Direction direction_;
  Alignment alignment_;
  Length length_;
  Length width_;
  bool no_padding_ = false;

  friend class Gui;
};

} // namespace datagui
