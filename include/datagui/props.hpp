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
  CheckboxBorderWidth,
  CheckboxBorderColor,
  CheckboxIconColor,
  CheckboxInnerPadding,

  // Dropdown
  DropdownInnerBorderWidth,

  // Float
  FloatType,

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

  void padding(const BoxDims& value) {
    props.insert(Prop::Padding, value);
  }
  void border_width(const BoxDims& value) {
    props.insert(Prop::BorderWidth, value);
  }
  void border_color(const Color& value) {
    props.insert(Prop::BorderColor, value);
  }
  void bg_color(const Color& value) {
    props.insert(Prop::BgColor, value);
  }
  void radius(float value) {
    props.insert(Prop::Radius, value);
  }

  void font(Font value) {
    props.insert(Prop::Font, value);
  }
  void font_size(int value) {
    props.insert(Prop::FontSize, value);
  }
  void text_color(const Color& value) {
    props.insert(Prop::TextColor, value);
  }
  void highlight_color(const Color& value) {
    props.insert(Prop::TextColor, value);
  }
  void cursor_width(int value) {
    props.insert(Prop::CursorWidth, value);
  }
  void cursor_color(const Color& value) {
    props.insert(Prop::CursorColor, value);
  }

  void layout_inner_padding(float value) {
    props.insert(Prop::LayoutInnerPadding, value);
  }
  void layout_direction(Direction value) {
    props.insert(Prop::LayoutDirection, value);
  }
  void layout_alignment(Alignment value) {
    props.insert(Prop::LayoutAlignment, value);
  }

  void focus_color(const Color& value) {
    props.insert(Prop::FocusColor, value);
  }
  void active_color(const Color& value) {
    props.insert(Prop::ActiveColor, value);
  }
  void hover_color(const Color& value) {
    props.insert(Prop::HoverColor, value);
  }

  void checkbox_size(float value) {
    props.insert(Prop::CheckboxSize, value);
  }
  void checkbox_border_width(const BoxDims& value) {
    props.insert(Prop::CheckboxBorderWidth, value);
  }
  void checkbox_border_color(const Color& value) {
    props.insert(Prop::CheckboxBorderColor, value);
  }
  void checkbox_icon_color(const Color& value) {
    props.insert(Prop::CheckboxIconColor, value);
  }
  void checkbox_inner_padding(const BoxDims& value) {
    props.insert(Prop::CheckboxInnerPadding, value);
  }

  void dropdown_inner_border_width(float value) {
    props.insert(Prop::DropdownInnerBorderWidth, value);
  }

  void float_type(FloatType value) {
    props.insert(Prop::FloatType, value);
  }

  void title_bar_enable(bool value) {
    props.insert(Prop::TitleBarEnable, value);
  }
  void title_bar_bg_color(const Color& value) {
    props.insert(Prop::TitleBarBgColor, value);
  }
  void title_bar_padding(const BoxDims& value) {
    props.insert(Prop::TitleBarPadding, value);
  }
  void title_bar_border_width(const BoxDims& value) {
    props.insert(Prop::TitleBarBorderWidth, value);
  }
  void title_bar_border_color(const Color& value) {
    props.insert(Prop::TitleBarBorderColor, value);
  }

  void close_button_enable(bool value) {
    props.insert(Prop::TitleBarEnable, value);
  }
  void close_button_color(const Color& value) {
    props.insert(Prop::CloseButtonColor, value);
  }
  void close_button_padding(const BoxDims& value) {
    props.insert(Prop::CloseButtonPadding, value);
  }

private:
  PropSet<Prop> props;
  friend class StyleManager;
};

class StyleManager {
public:
  void push(const Style& style) {
    props.push(style.props);
  }

  void down() {
    props.push_checkpoint();
  }

  void up() {
    props.pop_checkpoint();
  }

  const Length* width() {
    return props.get<Length>(Prop::Width);
  }
  const Length* height() {
    props.get<Length>(Prop::Height);
  }

  const BoxDims* padding() {
    return props.get<BoxDims>(Prop::Padding);
  }
  const BoxDims* border_width() {
    return props.get<BoxDims>(Prop::BorderWidth);
  }
  const Color* border_color() {
    return props.get<Color>(Prop::BorderColor);
  }
  const Color* bg_color() {
    return props.get<Color>(Prop::BgColor);
  }
  const float* radius() {
    return props.get<float>(Prop::Radius);
  }

  const Font* font() {
    return props.get<Font>(Prop::Font);
  }
  const int* font_size() {
    return props.get<int>(Prop::FontSize);
  }
  const Color* text_color() {
    return props.get<Color>(Prop::TextColor);
  }
  const Color* highlight_color() {
    return props.get<Color>(Prop::TextColor);
  }
  const int* cursor_width() {
    return props.get<int>(Prop::CursorWidth);
  }
  const Color* cursor_color() {
    return props.get<Color>(Prop::CursorColor);
  }

  const float* layout_inner_padding() {
    return props.get<float>(Prop::LayoutInnerPadding);
  }
  const Direction* layout_direction() {
    return props.get<Direction>(Prop::LayoutDirection);
  }
  const Alignment* layout_alignment() {
    return props.get<Alignment>(Prop::LayoutAlignment);
  }

  const Color* focus_color() {
    return props.get<Color>(Prop::FocusColor);
  }
  const Color* active_color() {
    return props.get<Color>(Prop::ActiveColor);
  }
  const Color* hover_color() {
    return props.get<Color>(Prop::HoverColor);
  }

  const float* checkbox_size() {
    return props.get<float>(Prop::CheckboxSize);
  }
  const BoxDims* checkbox_border_width() {
    return props.get<BoxDims>(Prop::CheckboxBorderWidth);
  }
  const Color* checkbox_border_color() {
    return props.get<Color>(Prop::CheckboxBorderColor);
  }
  const Color* checkbox_icon_color() {
    return props.get<Color>(Prop::CheckboxIconColor);
  }
  const BoxDims* checkbox_inner_padding() {
    return props.get<BoxDims>(Prop::CheckboxInnerPadding);
  }

  const float* dropdown_inner_border_width() {
    return props.get<float>(Prop::DropdownInnerBorderWidth);
  }

  const FloatType* float_type() {
    return props.get<FloatType>(Prop::FloatType);
  }

  const bool* title_bar_enable() {
    return props.get<bool>(Prop::TitleBarEnable);
  }
  const Color* title_bar_bg_color() {
    return props.get<Color>(Prop::TitleBarBgColor);
  }
  const BoxDims* title_bar_padding() {
    return props.get<BoxDims>(Prop::TitleBarPadding);
  }
  const BoxDims* title_bar_border_width() {
    return props.get<BoxDims>(Prop::TitleBarBorderWidth);
  }
  const Color* title_bar_border_color() {
    return props.get<Color>(Prop::TitleBarBorderColor);
  }

  const bool* close_button_enable() {
    return props.get<bool>(Prop::TitleBarEnable);
  }
  const Color* close_button_color() {
    return props.get<Color>(Prop::CloseButtonColor);
  }
  const BoxDims* close_button_padding() {
    return props.get<BoxDims>(Prop::CloseButtonPadding);
  }

private:
  PropStack<Prop> props;
};

} // namespace datagui
