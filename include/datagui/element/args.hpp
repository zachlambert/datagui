#pragma once

#include "datagui/element/tree.hpp"
#include <optional>

namespace datagui {

class Args {
  template <typename T>
  class Arg {
  public:
    Arg(T default_value = T()) :
        default_value(default_value), value(default_value) {}

    void operator=(const T& value) {
      this->value = value;
    }
    void operator=(T&& value) {
      this->value = std::move(value);
    }
    void consume(T& output) {
      output = value;
      value = default_value;
    }

  private:
    const T default_value;
    T value;
  };

  template <typename T>
  using ArgOpt = Arg<std::optional<T>>;

public:
  // Common

  void text_color(const Color& text_color) {
    text_color_ = text_color;
  }
  void text_size(int text_size) {
    text_size_ = text_size;
  }
  void bg_color(const Color& bg_color) {
    bg_color_ = bg_color;
  }
  void header_color(const Color& header_color) {
    header_color_ = header_color;
  }
  void border() {
    border_ = true;
  }
  void tight() {
    tight_ = true;
  }

  // Series

  void series_horizontal() {
    series_.direction = Direction::Horizontal;
  }
  void series_align_min() {
    series_.alignment = Alignment::Min;
  }
  void series_align_max() {
    series_.alignment = Alignment::Max;
  }

  void series_length_fixed(float size) {
    series_.length = LengthFixed(size);
  }
  void series_length_dynamic(float weight) {
    series_.length = LengthDynamic(weight);
  }

  void series_width_fixed(float size) {
    series_.width = LengthFixed(size);
  }
  void series_width_wrap() {
    series_.width = LengthWrap();
  }

private:
  void apply(ElementPtr element);

  ArgOpt<Color> text_color_;
  Arg<int> text_size_ = 0;
  ArgOpt<Color> bg_color_;
  ArgOpt<Color> header_color_;
  Arg<bool> border_ = false;
  Arg<bool> tight_ = false;

  struct {
    Arg<Direction> direction = Direction::Vertical;
    Arg<Alignment> alignment = Alignment::Center;
    Arg<Length> length = Length(LengthWrap());
    Arg<Length> width = Length(LengthDynamic(1));
  } series_;

  friend class Gui;
};

} // namespace datagui
