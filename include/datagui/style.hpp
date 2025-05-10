#pragma once

#include "datagui/color.hpp"
#include "datagui/font.hpp"
#include "datagui/layout.hpp"
#include "datagui/types/prop_stack.hpp"

namespace datagui {

enum class Prop {
  // Dimensions
  Width,
  Height,

  // Box
  Padding,
  BorderWidth,
  BorderColor,
  BgColor,
  Radius,

  // Text
  Font,
  FontSize,
  TextColor,
  HighlightColor,
  CursorWidth,
  CursorColor,

  // Layout
  LayoutInnerPadding,
  LayoutDirection,
  LayoutAlignment,

  // Input
  FocusColor,
  ActiveColor,
  HoverColor,

  // Checkbox
  CheckboxSize,
  CheckboxBgColor,
  CheckboxBorderWidth,
  CheckboxBorderColor,
  CheckboxIconColor,
  CheckboxInnerPadding,

  // Dropdown
  DropdownInnerBorderWidth,

  // Float
  FloatType,
  FloatBgColor,

  // TitleBar
  TitleBarEnable,
  TitleBarBgColor,
  TitleBarPadding,
  TitleBarBorderWidth,
  TitleBarBorderColor,

  // CloseButton
  CloseButtonEnable,
  CloseButtonColor,
  CloseButtonPadding,
};

#define STYLE_PROP(label, prop, type)                                          \
  void label(type&& value) {                                                   \
    props.insert<type>(prop, std::forward<type>(value));                       \
  }

class Style {
public:
  void width_fixed(float value) {
    props.insert<Length>(Prop::Width, LengthFixed(value));
  }
  void width_wrap() {
    props.insert<Length>(Prop::Width, LengthWrap());
  }
  void width_dynamic(float weight = 1) {
    props.insert<Length>(Prop::Width, LengthDynamic(weight));
  }

  void height_fixed(float value) {
    props.insert<Length>(Prop::Height, LengthFixed(value));
  }
  void height_wrap() {
    props.insert<Length>(Prop::Height, LengthWrap());
  }
  void height_dynamic(float weight = 1) {
    props.insert<Length>(Prop::Height, LengthDynamic(weight));
  }

  STYLE_PROP(padding, Prop::Padding, BoxDims)
  STYLE_PROP(border_width, Prop::BorderWidth, BoxDims)
  STYLE_PROP(border_color, Prop::BorderColor, Color)
  STYLE_PROP(bg_color, Prop::BgColor, Color)
  STYLE_PROP(radius, Prop::Radius, float)

  STYLE_PROP(font, Prop::Font, Font)
  STYLE_PROP(font_size, Prop::FontSize, int)
  STYLE_PROP(text_color, Prop::TextColor, Color)
  STYLE_PROP(highlight_color, Prop::HighlightColor, Color)
  STYLE_PROP(cursor_width, Prop::CursorWidth, float)
  STYLE_PROP(cursor_color, Prop::CursorColor, Color)

  STYLE_PROP(layout_inner_padding, Prop::LayoutInnerPadding, float)
  STYLE_PROP(layout_direction, Prop::LayoutDirection, Direction)
  STYLE_PROP(layout_alignment, Prop::LayoutAlignment, Alignment)

  STYLE_PROP(focus_color, Prop::FocusColor, Color)
  STYLE_PROP(active_color, Prop::ActiveColor, Color)
  STYLE_PROP(hover_color, Prop::HoverColor, Color)

  STYLE_PROP(checkbox_size, Prop::CheckboxSize, float)
  STYLE_PROP(checkbox_bg_color, Prop::CheckboxBgColor, Color)
  STYLE_PROP(checkbox_border_width, Prop::CheckboxBorderWidth, BoxDims)
  STYLE_PROP(checkbox_border_color, Prop::CheckboxBorderColor, Color)
  STYLE_PROP(checkbox_icon_color, Prop::CheckboxIconColor, Color)
  STYLE_PROP(checkbox_inner_padding, Prop::CheckboxInnerPadding, BoxDims)

  STYLE_PROP(dropdown_inner_border_width, Prop::DropdownInnerBorderWidth, float)

  void float_absolute(const BoxDims& margin) {
    props.insert<FloatType>(Prop::FloatType, FloatTypeAbsolute(margin));
  }
  void float_relative(const Vecf& offset, const Vecf& size) {
    props.insert<FloatType>(Prop::FloatType, FloatTypeRelative(offset, size));
  }
  STYLE_PROP(float_bg_color, Prop::FloatBgColor, Color)

  STYLE_PROP(title_bar_enable, Prop::TitleBarEnable, bool)
  STYLE_PROP(title_bar_bg_color, Prop::TitleBarBgColor, Color)
  STYLE_PROP(title_bar_bg_padding, Prop::TitleBarPadding, BoxDims)
  STYLE_PROP(title_bar_border_width, Prop::TitleBarBorderWidth, BoxDims)
  STYLE_PROP(title_bar_border_color, Prop::TitleBarBorderColor, Color)

  STYLE_PROP(close_button_enable, Prop::CloseButtonEnable, bool)
  STYLE_PROP(close_button_color, Prop::CloseButtonColor, Color)
  STYLE_PROP(close_button_padding, Prop::CloseButtonPadding, BoxDims)

private:
  PropSet<Prop> props;
  friend class StyleManager;
};
#undef STYLE_PROP

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

  STYLE_PROP(width, Prop::Width, Length)
  STYLE_PROP(height, Prop::Height, Length)

  STYLE_PROP(padding, Prop::Padding, BoxDims)
  STYLE_PROP(border_width, Prop::BorderWidth, BoxDims)
  STYLE_PROP(border_color, Prop::BorderColor, Color)
  STYLE_PROP(bg_color, Prop::BgColor, Color)
  STYLE_PROP(radius, Prop::Radius, float)

  STYLE_PROP(font, Prop::Font, Font)
  STYLE_PROP(font_size, Prop::FontSize, int)
  STYLE_PROP(text_color, Prop::TextColor, Color)
  STYLE_PROP(highlight_color, Prop::HighlightColor, Color)
  STYLE_PROP(cursor_width, Prop::CursorWidth, float)
  STYLE_PROP(cursor_color, Prop::CursorColor, Color)

  STYLE_PROP(layout_inner_padding, Prop::LayoutInnerPadding, float)
  STYLE_PROP(layout_direction, Prop::LayoutDirection, Direction)
  STYLE_PROP(layout_alignment, Prop::LayoutAlignment, Alignment)

  STYLE_PROP(focus_color, Prop::FocusColor, Color)
  STYLE_PROP(active_color, Prop::ActiveColor, Color)
  STYLE_PROP(hover_color, Prop::HoverColor, Color)

  STYLE_PROP(checkbox_size, Prop::CheckboxSize, float)
  STYLE_PROP(checkbox_bg_color, Prop::CheckboxBgColor, Color)
  STYLE_PROP(checkbox_border_width, Prop::CheckboxBorderWidth, BoxDims)
  STYLE_PROP(checkbox_border_color, Prop::CheckboxBorderColor, Color)
  STYLE_PROP(checkbox_icon_color, Prop::CheckboxIconColor, Color)
  STYLE_PROP(checkbox_inner_padding, Prop::CheckboxInnerPadding, BoxDims)

  STYLE_PROP(dropdown_inner_border_width, Prop::DropdownInnerBorderWidth, float)

  STYLE_PROP(float_type, Prop::FloatType, FloatType)
  STYLE_PROP(float_bg_color, Prop::FloatBgColor, Color)

  STYLE_PROP(title_bar_enable, Prop::TitleBarEnable, bool)
  STYLE_PROP(title_bar_bg_color, Prop::TitleBarBgColor, Color)
  STYLE_PROP(title_bar_padding, Prop::TitleBarPadding, BoxDims)
  STYLE_PROP(title_bar_border_width, Prop::TitleBarBorderWidth, BoxDims)
  STYLE_PROP(title_bar_border_color, Prop::TitleBarBorderColor, Color)

  STYLE_PROP(close_button_enable, Prop::CloseButtonEnable, bool)
  STYLE_PROP(close_button_color, Prop::CloseButtonColor, Color)
  STYLE_PROP(close_button_padding, Prop::CloseButtonPadding, BoxDims)

private:
  PropStack<Prop> props;
};
#undef STYLE_PROP

struct BoxStyle {
  BoxDims padding = BoxDims();
  BoxDims border_width = BoxDims();
  Color border_color = Color::Black();
  Color bg_color = Color::Clear();
  float radius = 0;
  Length width = literals::_wrap;
  Length height = literals::_wrap;

  void apply(const StyleManager& style) {
    style.padding(padding);
    style.border_width(border_width);
    style.border_color(border_color);
    style.bg_color(bg_color);
    style.radius(radius);
    style.width(width);
    style.height(height);
  }
};

struct TextStyle {
  Font font = Font::DejaVuSans;
  int font_size = 24;
  Color text_color = Color::Black();
  Color highlight_color = Color::Gray(0.7);
  float cursor_width = 2;
  Color cursor_color = Color::Gray(0.5);

  void apply(const StyleManager& style) {
    style.font(font);
    style.font_size(font_size);
    style.text_color(text_color);
    style.highlight_color(highlight_color);
    style.cursor_width(cursor_width);
    style.cursor_color(cursor_color);
  }
};

} // namespace datagui
