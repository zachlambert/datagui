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

  Args& text_color(const Color& text_color) {
    text_color_ = text_color;
    return *this;
  }
  Args& text_size(int text_size) {
    text_size_ = text_size;
    return *this;
  }
  Args& bg_color(const Color& bg_color) {
    bg_color_ = bg_color;
    return *this;
  }
  Args& header_color(const Color& header_color) {
    header_color_ = header_color;
    return *this;
  }
  Args& border() {
    border_ = true;
    return *this;
  }
  Args& tight() {
    tight_ = true;
    return *this;
  }
  Args& always() {
    always_ = true;
    return *this;
  }

  // Series

  Args& horizontal() {
    series_.direction = Direction::Horizontal;
    series_.length = LengthDynamic();
    series_.width = LengthWrap();
    return *this;
  }
  Args& align_center() {
    series_.alignment = Alignment::Center;
    return *this;
  }
  Args& align_max() {
    series_.alignment = Alignment::Max;
    return *this;
  }
  Args& length_fixed(float length) {
    series_.length = LengthFixed(length);
    return *this;
  }

  // Slider

  Args& slider_length(float length) {
    slider_.length = length;
    return *this;
  }

private:
  void apply(ElementPtr element);

  ArgOpt<Color> text_color_;
  Arg<int> text_size_ = 0;
  ArgOpt<Color> bg_color_;
  ArgOpt<Color> header_color_;
  Arg<bool> border_ = false;
  Arg<bool> tight_ = false;
  Arg<bool> always_ = false;

  struct {
    Arg<Direction> direction = Direction::Vertical;
    Arg<Alignment> alignment = Alignment::Min;
    Arg<Length> length = Length(LengthWrap());
    Arg<Length> width = Length(LengthDynamic(1));
  } series_;

  struct {
    ArgOpt<float> length;
  } slider_;

  friend class Gui;
};

} // namespace datagui
