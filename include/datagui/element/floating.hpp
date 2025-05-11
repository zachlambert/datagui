#pragma once

#include "datagui/resources.hpp"
#include "datagui/style.hpp"
#include "datagui/tree/element_system.hpp"

namespace datagui {

struct TitleBarStyle {
  Color bg_color = Color::Gray(0.8);
  BoxDims padding = 5;
  BoxDims border_width = 2;
  Color border_color = Color::Black();

  void apply(const StyleManager& style) {
    style.title_bar_bg_color(bg_color);
    style.title_bar_padding(padding);
    style.title_bar_border_width(border_width);
    style.title_bar_border_color(border_color);
  }
};

struct CloseButtonStyle {
  Color color = Color::Gray(0.6);
  BoxDims padding = 5;

  void apply(const StyleManager& style) {
    style.close_button_color(color);
  }
};

struct FloatingStyle {
  TextStyle text;
  FloatingType float_type = FloatingTypeAbsolute(BoxDims());
  Color bg_color;

  bool title_bar_enable = false;
  TitleBarStyle title_bar;

  bool close_button_enable = false;
  CloseButtonStyle close_button;

  void apply(const StyleManager& style) {
    text.apply(style);
    style.floating_type(float_type);
    style.floating_bg_color(bg_color);

    style.title_bar_enable(title_bar_enable);
    title_bar.apply(style);

    style.close_button_enable(close_button_enable);
    close_button.apply(style);
  }
};

struct FloatingData {
  FloatingStyle style;
  std::string title;
  bool open = false;
  bool open_changed = false;
  Boxf title_bar_box;
  float title_bar_text_width;
  Boxf close_button_box;
};

class FloatingSystem : public ElementSystemImpl<FloatingData> {
public:
  FloatingSystem(Resources& res) : res(res) {}

  void visit(Element element, Variable<bool> open, const std::string& title);

  void set_input_state(Element element) const override;
  void set_dependent_state(Element elment) const override;
  void render(ConstElement element) const override;
  void mouse_event(Element element, const MouseEvent& event) override;

private:
  Resources& res;
};

} // namespace datagui
