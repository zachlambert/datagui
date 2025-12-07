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

    T& operator*() {
      return value;
    }
    T* operator->() {
      return &value;
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
    *text_color_ = text_color;
    return *this;
  }
  Args& text_size(int text_size) {
    *text_size_ = text_size;
    return *this;
  }
  Args& bg_color(const Color& bg_color) {
    *bg_color_ = bg_color;
    return *this;
  }
  Args& header_color(const Color& header_color) {
    *header_color_ = header_color;
    return *this;
  }
  Args& border() {
    *border_ = true;
    return *this;
  }
  Args& always() {
    *always_ = true;
    return *this;
  }
  Args& label(const std::string& label) {
    *label_ = label;
    return *this;
  }

  // Layout

  Args& vertical() {
    layout_->rows = -1;
    layout_->cols = 1;
    layout_->x_alignment = XAlignment::Left;
    layout_->y_alignment = YAlignment::Center;
    return *this;
  }
  Args& horizontal() {
    layout_->rows = 1;
    layout_->cols = -1;
    layout_->x_alignment = XAlignment::Left;
    layout_->y_alignment = YAlignment::Center;
    return *this;
  }
  Args& grid(int rows, int cols) {
    if (rows == -1 && cols == -1) {
      cols = 1;
    }
    layout_->rows = rows;
    layout_->cols = cols;
    layout_->x_alignment = XAlignment::Left;
    layout_->y_alignment = YAlignment::Center;
    return *this;
  }
  Args& align_left() {
    layout_->x_alignment = XAlignment::Left;
    return *this;
  }
  Args& align_center_h() {
    layout_->x_alignment = XAlignment::Left;
    return *this;
  }
  Args& align_right() {
    layout_->x_alignment = XAlignment::Right;
    return *this;
  }

  Args& width_fixed(float length) {
    *width_ = LengthFixed(length);
    return *this;
  }
  Args& height_fixed(float length) {
    *height_ = LengthFixed(length);
    return *this;
  }
  Args& width_expand(float weight = 1) {
    *width_ = LengthDynamic(weight);
    return *this;
  }
  Args& height_expand(float weight = 1) {
    *height_ = LengthDynamic(weight);
    return *this;
  }

  Args& tight() {
    layout_->tight = true;
    return *this;
  }

  // Other

  Args& slider_length(float length) {
    *slider_length_ = length;
    return *this;
  }

  Args& split_fixed() {
    *split_fixed_ = true;
    return *this;
  }

  Args& text_input_expand(float weight = 1) {
    *text_input_width_ = LengthDynamic(weight);
    return *this;
  }
  Args& text_input_width(float width) {
    *text_input_width_ = LengthFixed(width);
    return *this;
  }

  Args& dropdown_horizontal() {
    *dropdown_direction_ = Direction::Horizontal;
    return *this;
  }

private:
  void apply(ElementPtr element);

  ArgOpt<Color> text_color_;
  Arg<int> text_size_ = 0;
  ArgOpt<Color> bg_color_;
  ArgOpt<Color> header_color_;
  Arg<bool> border_ = false;
  Arg<bool> always_ = false;
  Arg<std::string> label_ = {};
  Arg<Length> width_ = Length(LengthWrap());
  Arg<Length> height_ = Length(LengthWrap());
  Arg<Layout> layout_ = Layout();
  ArgOpt<float> slider_length_;
  Arg<bool> split_fixed_ = false;
  ArgOpt<Length> text_input_width_;
  Arg<Direction> dropdown_direction_ = Direction::Vertical;

  friend class Gui;
};

} // namespace datagui
