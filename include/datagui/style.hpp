#pragma once

#include "datagui/color.hpp"
#include "datagui/font.hpp"
#include "datagui/layout.hpp"
#include <assert.h>
#include <functional>
#include <memory>

namespace datagui {

class StyleRuleTBase {
public:
  virtual ~StyleRuleTBase() = 0;
};

class StyleRule;

template <typename Style>
class StyleRuleT : public StyleRuleTBase {
public:
  template <typename Functor>
  StyleRuleT(const Functor& func) : func(func) {}
  template <typename Functor>
  StyleRuleT(Functor&& func) : func(std::move(func)) {}

  StyleRuleT(const std::function<void(Style&)>& func) : func(func) {}
  StyleRuleT(std::function<void(Style&)>&& func) : func(std::move(func)) {}

  template <typename Other>
  StyleRule join(const StyleRuleT<Other>& other);

  void apply(Style& style) const {
    func(style);
  }

private:
  std::function<void(Style&)> func;
};

class StyleRule {
public:
  StyleRule() = default;

  template <typename Style>
  StyleRule(const StyleRuleT<Style>& style) : styles{style} {}

  template <typename Style>
  StyleRule& join(const StyleRuleT<Style>& style) {
    styles.push_back(style);
    return *this;
  }

  template <typename Style>
  void apply(Style& style) const {
    for (const auto& style : styles) {
      auto style_t = dynamic_cast<const StyleRuleT<Style>*>(style.get());
      if (style_t) {
        style_t->apply(style);
      }
    }
  }

private:
  std::vector<std::unique_ptr<StyleRuleTBase>> styles;
};

template <typename Style>
template <typename Other>
StyleRule StyleRuleT<Style>::join(const StyleRuleT<Other>& other) {
  return StyleRule().join(other);
}

class StyleManager {
public:
  StyleManager() {
    rules.emplace_back();
  }
  void push(const StyleRule& rule) {
    assert(!rules.empty());
    rules.back().push_back(rule);
  }
  void pop() {
    assert(!rules.empty());
    assert(!rules.back().empty());
    rules.back().pop_back();
  }
  void down() {
    rules.emplace_back();
  }
  void up() {
    rules.pop_back();
  }

  template <typename Style>
  void apply(Style& style) {
    for (const auto& rules_layer : rules) {
      for (const auto& rule : rules_layer) {
        rule.apply(style);
      }
    }
  }

private:
  std::vector<std::vector<StyleRule>> rules;
};

struct BoxStyle {
  BoxDims padding = BoxDims();
  BoxDims border_width = BoxDims();
  Color border_color = Color::Black();
  Color bg_color = Color::Clear();
  float radius = 0;
};

StyleRuleT<BoxStyle> box_padding(const BoxDims& value) {
  return StyleRuleT<BoxStyle>(
      [value](BoxStyle& style) { style.padding = value; });
};

struct LinearLayoutStyle : public BoxStyle {
  Length length = literals::_wrap;
  Length width = literals::_wrap;
  float inner_padding;
};

struct TextStyle {
  Font font = Font::DejaVuSans;
  int font_size = 24;
  Color text_color = Color::Black();
};

struct SelectableTextStyle : public TextStyle {
  float cursor_width = 2;
  Color cursor_color = Color::Gray(0.5);
  Color highlight_color = Color::Gray(0.7);
  bool disabled = false;
};

} // namespace datagui
