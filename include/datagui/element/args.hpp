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
    return *this;
  }
  Args& horizontal() {
    layout_->rows = 1;
    layout_->cols = -1;
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

  Args& group_width_fixed(float length) {
    group_fixed_size_->x = length;
    return *this;
  }
  Args& group_height_fixed(float length) {
    group_fixed_size_->y = length;
    return *this;
  }

  Args& tight() {
    layout_->tight = true;
    return *this;
  }

  // Slider

  Args& slider_length(float length) {
    *slider_length_ = length;
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
  Arg<Vec2> group_fixed_size_ = {};
  Arg<Layout> layout_ = {};
  ArgOpt<float> slider_length_;

  friend class Gui;
};

} // namespace datagui
