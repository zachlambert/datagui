#pragma once

#include "datagui/style.hpp"
#include "datagui/tree/element.hpp"

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

struct FloatingProps {
  // Style
  TextStyle text_style;
  FloatingType float_type = FloatingTypeAbsolute(BoxDims());
  Color bg_color;
  bool title_bar_enable = false;
  TitleBarStyle title_bar;
  bool close_button_enable = false;
  CloseButtonStyle close_button;

  // Data
  std::string title;
  bool open = false;
  bool open_changed = false;
  Boxf title_bar_box;
  float title_bar_text_width;
  Boxf close_button_box;

  void set_style(const StyleManager& style) {
    text_style.apply(style);
    style.floating_type(float_type);
    style.floating_bg_color(bg_color);

    style.title_bar_enable(title_bar_enable);
    title_bar.apply(style);

    style.close_button_enable(close_button_enable);
    close_button.apply(style);
  }
};

class FloatingSystem : public ElementSystem {
public:
  FloatingSystem(std::shared_ptr<FontManager> fm) : fm(fm) {}

  void set_input_state(Element& element, const ConstElementList& children)
      override;
  void set_dependent_state(Element& element, const ElementList& children)
      override;
  void render(const Element& element, Renderer& renderer) override;
  bool mouse_event(Element& element, const MouseEvent& event) override;

private:
  std::shared_ptr<FontManager> fm;
};

inline FloatingProps& get_floating(
    ElementSystemList& systems,
    Element& element,
    bool initial_open_value) {
  if (element.system == -1) {
    element.system = systems.find<FloatingSystem>();
    assert(element.system != -1);
    assert(!element.props);
    element.props = UniqueAny::Make<FloatingProps>();
    auto props = *element.props.cast<FloatingProps>();
    props.open = initial_open_value;
  }
  auto props = element.props.cast<FloatingProps>();
  assert(props);
  return *props;
}

} // namespace datagui
