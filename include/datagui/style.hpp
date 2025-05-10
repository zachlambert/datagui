#pragma once

#include "datagui/color.hpp"
#include "datagui/font.hpp"
#include "datagui/layout.hpp"
#include "datagui/types/prop_stack.hpp"

namespace datagui {

enum class Prop {
  // =============================
  // Generic

  // Text
  Font,
  FontSize,
  TextColor,
  TextPadding,
  HighlightColor,
  CursorWidth,
  CursorColor,

  // =============================
  // Layout Elements

  // Series
  SeriesDirection,
  SeriesAlignment,
  SeriesLength,
  SeriesWidth,
  SeriesBgColor,
  SeriesOuterPadding,
  SeriesInnerPadding,

  // =============================
  // Primitive Elements

  // Generic Input
  FocusColorFactor,
  ActiveColorFactor,
  HoverColorFactor,

  // Button
  ButtonWidth,
  ButtonBgColor,
  ButtonBorderWidth,
  ButtonBorderColor,
  ButtonRadius,

  // Checkbox
  CheckboxSize,
  CheckboxBgColor,
  CheckboxBorderWidth,
  CheckboxBorderColor,
  CheckboxIconColor,
  CheckboxInnerPadding,
  CheckboxRadius,

  // Dropdown
  DropdownWidth,
  DropdownBgColor,
  DropdownBorderWidth,
  DropdownBorderColor,
  DropdownInnerBorderWidth,
  DropdownInnerBorderColor,

  // TextInput
  TextInputWidth,
  TextInputBgColor,
  TextInputBorderWidth,
  TextInputBorderColor,

  // =============================
  // Floating

  // Floating
  FloatingType,
  FloatingBgColor,

  // TitleBar
  TitleBarEnable,
  TitleBarBgColor,
  TitleBarPadding,
  TitleBarBorderWidth,
  TitleBarBorderColor,

  // CloseButton
  CloseButtonEnable,
  CloseButtonColor,
};

#define STYLE_PROP(label, prop, type)                                          \
  Style& label(type&& value) {                                                 \
    props.insert<type>(prop, std::forward<type>(value));                       \
    return *this;                                                              \
  }

#define STYLE_LENGTH(label, prop)                                              \
  Style& label##_fixed(float value) {                                          \
    props.insert<Length>(prop, LengthFixed(value));                            \
    return *this;                                                              \
  }                                                                            \
  Style& label##_wrap() {                                                      \
    props.insert<Length>(prop, LengthWrap());                                  \
    return *this;                                                              \
  }                                                                            \
  Style& label##_dynamic(float weight = 1) {                                   \
    props.insert<Length>(prop, LengthDynamic(weight));                         \
    return *this;                                                              \
  }

#define STYLE_VALUE(label, prop, type, value)                                  \
  Style& label() {                                                             \
    props.insert<type>(prop, value);                                           \
    return *this;                                                              \
  }

class Style {
public:
  // Generic

  STYLE_PROP(font, Prop::Font, Font)
  STYLE_PROP(font_size, Prop::FontSize, int)
  STYLE_PROP(text_color, Prop::TextColor, Color)
  STYLE_PROP(text_padding, Prop::TextPadding, BoxDims)
  STYLE_PROP(highlight_color, Prop::HighlightColor, Color)
  STYLE_PROP(cursor_width, Prop::CursorWidth, float)
  STYLE_PROP(cursor_color, Prop::CursorColor, Color)

  // Layout Elements

  STYLE_VALUE(
      series_horizontal,
      Prop::SeriesDirection,
      Direction,
      Direction::Horizontal)
  STYLE_VALUE(
      series_vertical,
      Prop::SeriesDirection,
      Direction,
      Direction::Vertical)
  STYLE_VALUE(
      series_align_center,
      Prop::SeriesAlignment,
      Alignment,
      Alignment::Center)
  STYLE_VALUE(
      series_align_min,
      Prop::SeriesAlignment,
      Alignment,
      Alignment::Min)
  STYLE_VALUE(
      series_align_max,
      Prop::SeriesAlignment,
      Alignment,
      Alignment::Max)
  STYLE_LENGTH(layout_length, Prop::SeriesLength)
  STYLE_LENGTH(layout_width, Prop::SeriesWidth)
  STYLE_PROP(series_bg_color, Prop::SeriesBgColor, Color)
  STYLE_PROP(series_outer_padding, Prop::SeriesOuterPadding, BoxDims)
  STYLE_PROP(series_inner_padding, Prop::SeriesInnerPadding, float)

  // Primitive Elements

  STYLE_PROP(focus_color_factor, Prop::FocusColorFactor, float)
  STYLE_PROP(active_color_factor, Prop::ActiveColorFactor, float)
  STYLE_PROP(hover_color_factor, Prop::HoverColorFactor, float)

  STYLE_LENGTH(button_width, Prop::ButtonWidth)
  STYLE_PROP(button_bg_color, Prop::ButtonBgColor, Color)
  STYLE_PROP(button_border_width, Prop::ButtonBorderWidth, BoxDims)
  STYLE_PROP(button_border_color, Prop::ButtonBorderColor, Color)
  STYLE_PROP(button_radius, Prop::ButtonRadius, float)

  STYLE_PROP(checkbox_size, Prop::CheckboxSize, float)
  STYLE_PROP(checkbox_bg_color, Prop::CheckboxBgColor, Color)
  STYLE_PROP(checkbox_border_width, Prop::CheckboxBorderWidth, BoxDims)
  STYLE_PROP(checkbox_border_color, Prop::CheckboxBorderColor, Color)
  STYLE_PROP(checkbox_icon_color, Prop::CheckboxIconColor, Color)
  STYLE_PROP(checkbox_inner_padding, Prop::CheckboxInnerPadding, BoxDims)
  STYLE_PROP(checkbox_radius, Prop::CheckboxRadius, float)

  STYLE_LENGTH(dropdown_width, Prop::DropdownWidth)
  STYLE_PROP(dropdown_bg_color, Prop::DropdownBgColor, Color)
  STYLE_PROP(dropdown_border_width, Prop::DropdownBorderWidth, BoxDims)
  STYLE_PROP(dropdown_border_color, Prop::DropdownBorderColor, Color)
  STYLE_PROP(dropdown_inner_border_width, Prop::DropdownInnerBorderWidth, float)
  STYLE_PROP(dropdown_inner_border_color, Prop::DropdownInnerBorderColor, Color)

  STYLE_LENGTH(text_input_width, Prop::TextInputWidth)
  STYLE_PROP(text_input_bg_color, Prop::TextInputBgColor, Color)
  STYLE_PROP(text_input_border_width, Prop::TextInputBorderWidth, BoxDims)
  STYLE_PROP(text_input_border_color, Prop::TextInputBorderColor, Color)

  // Floating

  void float_absolute(const BoxDims& margin) {
    props.insert<FloatingType>(
        Prop::FloatingType,
        FloatingTypeAbsolute(margin));
  }
  void float_relative(const Vecf& offset, const Vecf& size) {
    props.insert<FloatingType>(
        Prop::FloatingType,
        FloatingTypeRelative(offset, size));
  }
  STYLE_PROP(float_bg_color, Prop::FloatingBgColor, Color)

  STYLE_PROP(title_bar_enable, Prop::TitleBarEnable, bool)
  STYLE_PROP(title_bar_bg_color, Prop::TitleBarBgColor, Color)
  STYLE_PROP(title_bar_padding, Prop::TitleBarPadding, BoxDims)
  STYLE_PROP(title_bar_border_width, Prop::TitleBarBorderWidth, BoxDims)
  STYLE_PROP(title_bar_border_color, Prop::TitleBarBorderColor, Color)

  STYLE_PROP(close_button_enable, Prop::CloseButtonEnable, bool)
  STYLE_PROP(close_button_color, Prop::CloseButtonColor, Color)

private:
  PropSet<Prop> props;
  friend class StyleManager;
};
#undef STYLE_PROP
#undef STYLE_LENGTH

#define STYLE_PROP(name, prop, type)                                           \
  void name(type& value) const {                                               \
    if (auto ptr = props.get<type>(prop)) {                                    \
      value = *ptr;                                                            \
    }                                                                          \
  }

class StyleManager {
public:
  void push(const Style& style) {
    props.push(style.props);
  }

  void push_temp(const Style& style) {
    props.push_checkpoint();
    props.push(style.props);
  }

  void pop_temp() {
    props.pop_checkpoint();
  }

  void down() {
    props.push_checkpoint();
  }

  void up() {
    props.pop_checkpoint();
  }

  // Generic

  STYLE_PROP(font, Prop::Font, Font)
  STYLE_PROP(font_size, Prop::FontSize, int)
  STYLE_PROP(text_color, Prop::TextColor, Color)
  STYLE_PROP(text_padding, Prop::TextPadding, BoxDims)
  STYLE_PROP(highlight_color, Prop::HighlightColor, Color)
  STYLE_PROP(cursor_width, Prop::CursorWidth, float)
  STYLE_PROP(cursor_color, Prop::CursorColor, Color)

  // Layout Elements

  STYLE_PROP(series_direction, Prop::SeriesDirection, Direction)
  STYLE_PROP(series_alignment, Prop::SeriesAlignment, Alignment)
  STYLE_PROP(series_length, Prop::SeriesLength, Length)
  STYLE_PROP(series_width, Prop::SeriesWidth, Length)
  STYLE_PROP(series_bg_color, Prop::SeriesBgColor, Color)
  STYLE_PROP(series_outer_padding, Prop::SeriesOuterPadding, BoxDims)
  STYLE_PROP(series_inner_padding, Prop::SeriesInnerPadding, float)

  // Primitive elements

  STYLE_PROP(focus_color_factor, Prop::FocusColorFactor, float)
  STYLE_PROP(active_color_factor, Prop::ActiveColorFactor, float)
  STYLE_PROP(hover_color_factor, Prop::HoverColorFactor, float)

  STYLE_PROP(button_width, Prop::ButtonWidth, Length)
  STYLE_PROP(button_bg_color, Prop::ButtonBgColor, Color)
  STYLE_PROP(button_border_width, Prop::ButtonBorderWidth, BoxDims)
  STYLE_PROP(button_border_color, Prop::ButtonBorderColor, Color)
  STYLE_PROP(button_radius, Prop::ButtonRadius, float)

  STYLE_PROP(checkbox_size, Prop::CheckboxSize, float)
  STYLE_PROP(checkbox_bg_color, Prop::CheckboxBgColor, Color)
  STYLE_PROP(checkbox_border_width, Prop::CheckboxBorderWidth, BoxDims)
  STYLE_PROP(checkbox_border_color, Prop::CheckboxBorderColor, Color)
  STYLE_PROP(checkbox_icon_color, Prop::CheckboxIconColor, Color)
  STYLE_PROP(checkbox_inner_padding, Prop::CheckboxInnerPadding, BoxDims)
  STYLE_PROP(checkbox_radius, Prop::CheckboxRadius, float)

  STYLE_PROP(dropdown_width, Prop::DropdownWidth, Length)
  STYLE_PROP(dropdown_bg_color, Prop::DropdownBgColor, Color)
  STYLE_PROP(dropdown_border_width, Prop::DropdownBorderWidth, BoxDims)
  STYLE_PROP(dropdown_border_color, Prop::DropdownBorderColor, Color)
  STYLE_PROP(dropdown_inner_border_width, Prop::DropdownInnerBorderWidth, float)
  STYLE_PROP(dropdown_inner_border_color, Prop::DropdownInnerBorderColor, Color)

  STYLE_PROP(text_input_width, Prop::TextInputWidth, Length)
  STYLE_PROP(text_input_bg_color, Prop::TextInputBgColor, Color)
  STYLE_PROP(text_input_border_width, Prop::TextInputBorderWidth, BoxDims)
  STYLE_PROP(text_input_border_color, Prop::TextInputBorderColor, Color)

  // Floating

  STYLE_PROP(floating_type, Prop::FloatingType, FloatingType)
  STYLE_PROP(floating_bg_color, Prop::FloatingBgColor, Color)

  STYLE_PROP(title_bar_enable, Prop::TitleBarEnable, bool)
  STYLE_PROP(title_bar_bg_color, Prop::TitleBarBgColor, Color)
  STYLE_PROP(title_bar_padding, Prop::TitleBarPadding, BoxDims)
  STYLE_PROP(title_bar_border_width, Prop::TitleBarBorderWidth, BoxDims)
  STYLE_PROP(title_bar_border_color, Prop::TitleBarBorderColor, Color)

  STYLE_PROP(close_button_enable, Prop::CloseButtonEnable, bool)
  STYLE_PROP(close_button_color, Prop::CloseButtonColor, Color)

private:
  PropStack<Prop> props;
};
#undef STYLE_PROP

struct TextStyle {
  Font font = Font::DejaVuSans;
  int font_size = 24;
  Color text_color = Color::Black();
  Color highlight_color = Color::Clear();
  float cursor_width = 2;
  Color cursor_color = Color::Clear();

  void apply(const StyleManager& style) {
    style.font(font);
    style.font_size(font_size);
    style.text_color(text_color);
    style.highlight_color(highlight_color);
    style.cursor_width(cursor_width);
    style.cursor_color(cursor_color);
  }
};

struct InputStyle {
  float active_color_factor = 1;
  float hover_color_factor = 1;
  float focus_color_factor = 1;

  void apply(const StyleManager& style) {
    style.active_color_factor(active_color_factor);
    style.hover_color_factor(hover_color_factor);
    style.focus_color_factor(focus_color_factor);
  }
};

} // namespace datagui
