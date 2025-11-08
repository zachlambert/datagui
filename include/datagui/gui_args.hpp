#pragma once

#include "datagui/color.hpp"
#include "datagui/layout.hpp"
#include <optional>

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

  void bg_color(const Color& bg_color) {
    bg_color_ = bg_color;
  }

private:
  void apply(SeriesProps& props) const;

  void reset() {
    direction_ = Direction::Vertical;
    alignment_ = Alignment::Center;
    length_ = LengthWrap();
    width_ = LengthDynamic(1);
    no_padding_ = false;
    bg_color_ = std::nullopt;
  }

  Direction direction_;
  Alignment alignment_;
  Length length_;
  Length width_;
  bool no_padding_;
  std::optional<Color> bg_color_;

  friend class Gui;
};

class FloatingProps;

class FloatingArgs {
public:
  FloatingArgs() {
    reset();
  }

  void bg_color(const Color& bg_color) {
    bg_color_ = bg_color;
  }

private:
  void apply(FloatingProps& props) const;

  void reset() {
    bg_color_ = std::nullopt;
  }

  std::optional<Color> bg_color_;

  friend class Gui;
};

} // namespace datagui
