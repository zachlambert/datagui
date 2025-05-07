#pragma once

#include "datagui/color.hpp"
#include "datagui/font.hpp"
#include "datagui/layout.hpp"
#include <assert.h>
#include <functional>
#include <memory>

namespace datagui {

struct BoxStyle {
  BoxDims padding = BoxDims();
  BoxDims border_width = BoxDims();
  Color border_color = Color::Black();
  Color bg_color = Color::Clear();
  float radius = 0;
  Length width = literals::_wrap;
  Length height = literals::_wrap;
};

enum class LayoutDirection { Vertical, Horizontal };

enum class LayoutAlignment { Top, Bottom, Left, Right };

struct LayoutStyle : public BoxStyle {
  float inner_padding;
  LayoutDirection direction;
  LayoutAlignment alignment;
};

struct TextStyle {
  Font font = Font::DejaVuSans;
  int font_size = 24;
  Color text_color = Color::Black();
  Color highlight_color = Color::Gray(0.7);
  float cursor_width = 2;
  Color cursor_color = Color::Gray(0.5);
};

struct InputStyle {
  Color focus_color = Color(0, 1, 1);
  Color active_color = Color::Gray(0.4);
  Color hover_color = Color::Gray(0.6);
};

struct ButtonStyle : public BoxStyle, public TextStyle, public InputStyle {};

struct CheckboxStyle : public BoxStyle, InputStyle {
  Color check_color = Color::Gray(0.2);
  BoxDims check_padding = 2;
};

struct DropdownStyle : public BoxStyle, public TextStyle {
  float inner_border_width = 0;
};

struct TextInputStyle : public BoxStyle, public TextStyle, InputStyle {};

struct FloatTypeAbsolute {
  BoxDims margin;
  FloatTypeAbsolute(const BoxDims& margin) : margin(margin) {}
};

struct FloatTypeRelative {
  Vecf offset;
  Vecf size;
  FloatTypeRelative(const Vecf& offset, const Vecf& size) :
      offset(offset), size(size) {}
};

using FloatType = std::variant<FloatTypeAbsolute, FloatTypeRelative>;

struct BarStyle : public TextStyle {
  Color bg_color = Color::Gray(0.8);
  bool close_button_enable = true;
  Color close_button_color = Color::Gray(0.6);
  BoxDims padding = 5;
  BoxDims border_width = 0;
  Color border_color = Color::Black();
};

struct FloatStyle : public TextStyle {
  FloatType type;
  bool bar_enable = false;
  BarStyle bar;
};

class StyleRuleTBase {
public:
  virtual ~StyleRuleTBase() = 0;
};

template <typename Style>
class StyleRuleT : public StyleRuleTBase {
public:
  template <typename Functor>
  StyleRuleT(const Functor& func) : func(func) {}
  template <typename Functor>
  StyleRuleT(Functor&& func) : func(std::move(func)) {}

  StyleRuleT(const std::function<void(Style&)>& func) : func(func) {}
  StyleRuleT(std::function<void(Style&)>&& func) : func(std::move(func)) {}

  void apply(Style& style) const {
    func(style);
  }

private:
  std::function<void(Style&)> func;
};

#define STYLE_PROPERTY(label, StyleT, name)                                    \
  Style& label(const typeof(StyleT::name)& name) {                             \
    styles.push_back(std::make_unique<StyleRuleT<StyleT>>(                     \
        [name](StyleT& style) { style.name = name; }));                        \
    return *this;                                                              \
  }

#define STYLE_SUB_PROPERTY(label, StyleT, name, subname)                       \
  Style& label(const typeof(StyleT::name.subname)& name##_##subname) {         \
    styles.push_back(std::make_unique<StyleRuleT<StyleT>>(                     \
        [name##_##subname](StyleT& style) {                                    \
          style.name##_##enable = true;                                        \
          style.name.subname = name##_##subname;                               \
        }));                                                                   \
    return *this;                                                              \
  }

class Style {
public:
  template <typename Style>
  void apply(Style& style) const {
    for (const auto& style : styles) {
      auto style_t = dynamic_cast<const StyleRuleT<Style>*>(style.get());
      if (style_t) {
        style_t->apply(style);
      }
    }
  }

  // BoxStyle
  STYLE_PROPERTY(padding, BoxStyle, padding)
  STYLE_PROPERTY(border_width, BoxStyle, border_width)
  STYLE_PROPERTY(border_color, BoxStyle, border_color)
  STYLE_PROPERTY(bg_color, BoxStyle, bg_color)
  STYLE_PROPERTY(radius, BoxStyle, radius)
  STYLE_PROPERTY(width, BoxStyle, width)
  STYLE_PROPERTY(height, BoxStyle, height)

  // LinearLayoutStyle
  STYLE_PROPERTY(layout_direction, LayoutStyle, direction)
  STYLE_PROPERTY(layout_alignment, LayoutStyle, alignment)
  STYLE_PROPERTY(layout_inner_padding, LayoutStyle, inner_padding)

  // TextStyle
  STYLE_PROPERTY(font, TextStyle, font)
  STYLE_PROPERTY(font_size, TextStyle, font_size)
  STYLE_PROPERTY(text_color, TextStyle, text_color)
  STYLE_PROPERTY(text_highlight_color, TextStyle, highlight_color)
  STYLE_PROPERTY(cursor_width, TextStyle, cursor_width)
  STYLE_PROPERTY(cursor_color, TextStyle, cursor_color)

  // InputStyle
  STYLE_PROPERTY(input_focus_color, InputStyle, focus_color)
  STYLE_PROPERTY(input_active_color, InputStyle, active_color)
  STYLE_PROPERTY(input_hover_color, InputStyle, hover_color)

  // CheckboxStyle
  STYLE_PROPERTY(checkbox_check_color, CheckboxStyle, check_color)
  STYLE_PROPERTY(checkbox_check_padding, CheckboxStyle, check_padding)

  // CheckboxStyle
  STYLE_PROPERTY(dropdown_inner_border_width, DropdownStyle, inner_border_width)

  // FloatStyle

  Style& float_absolute(const BoxDims& margin) {
    styles.push_back(
        std::make_unique<StyleRuleT<FloatStyle>>([margin](FloatStyle& style) {
          style.type = FloatTypeAbsolute(margin);
        }));
    return *this;
  }

  Style& float_relative(const Vecf& offset, const Vecf& size) {
    styles.push_back(std::make_unique<StyleRuleT<FloatStyle>>(
        [offset, size](FloatStyle& style) {
          style.type = FloatTypeRelative(offset, size);
        }));
    return *this;
  }

  STYLE_SUB_PROPERTY(float_bar_bg_color, FloatStyle, bar, bg_color)
  STYLE_SUB_PROPERTY(
      float_bar_close_button_enable,
      FloatStyle,
      bar,
      close_button_enable)
  STYLE_SUB_PROPERTY(
      float_bar_close_button_color,
      FloatStyle,
      bar,
      close_button_color)
  STYLE_SUB_PROPERTY(float_bar_padding, FloatStyle, bar, padding)
  STYLE_SUB_PROPERTY(float_bar_border_width, FloatStyle, bar, border_width)
  STYLE_SUB_PROPERTY(float_bar_border_color, FloatStyle, bar, border_color)

private:
  std::vector<std::unique_ptr<StyleRuleTBase>> styles;
};

class StyleManager {
public:
  StyleManager() {
    styles.emplace_back();
  }
  void push(const Style& style) {
    assert(!styles.empty());
    styles.back().push_back(style);
  }
  void pop() {
    assert(!styles.empty());
    assert(!styles.back().empty());
    styles.back().pop_back();
  }
  void down() {
    styles.emplace_back();
  }
  void up() {
    styles.pop_back();
  }

  template <typename Style>
  void apply(Style& style) {
    for (const auto& rules_layer : styles) {
      for (const auto& rule : rules_layer) {
        rule.apply(style);
      }
    }
  }

private:
  std::vector<std::vector<Style>> styles;
};

} // namespace datagui
