#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/resources.hpp"
#include "datagui/tree/tree.hpp"
#include "datagui/visual/window.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/dropdown.hpp"
#include "datagui/element/floating.hpp"
#include "datagui/element/horizontal_layout.hpp"
#include "datagui/element/text_box.hpp"
#include "datagui/element/text_input.hpp"
#include "datagui/element/vertical_layout.hpp"

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  void set_debug_mode(bool value) {
    debug_mode_ = value;
  }
  bool running() const;
  void begin();
  void end();

  bool horizontal_layout(const Style& = Style());
  bool vertical_layout(const Style& = Style());
  void layout_end();

  bool button(const std::string& text, const Style& style = Style());

  const bool* checkbox(
      const bool& initial_checked = false,
      const Style& style = Style());

  void checkbox(const Variable<bool>& checked, const Style& style = Style());

  void text_box(const std::string& text, const Style& style = Style());

  const std::string* text_input(
      const std::string& initial_text = "",
      const Style& style = Style());

  void text_input(
      const Variable<std::string>& text,
      const Style& style = Style());

  const int* dropdown(
      const std::vector<std::string>& choices,
      int initial_choice = -1,
      const Style& style = Style());

  void dropdown(
      const std::vector<std::string>& choices,
      const Variable<int>& choice,
      const Style& style = Style());

  bool floating(
      const Variable<bool>& open,
      const std::string& title,
      const Style& style = Style());

  template <typename T>
  Variable<T> variable(const T& initial_value = T()) {
    // Capture initial_value by value
    return tree.variable<T>([initial_value]() { return initial_value; });
  }

  void style(const Style& style) {
    res.style_manager.push(style);
  }

private:
  void render();
  void calculate_sizes();

  void event_handling();
  Element get_leaf_node(const Vecf& position);
  void event_handling_left_click(const MouseEvent& event);
  void event_handling_hover(const Vecf& mouse_pos);
  void event_handling_scroll(const ScrollEvent& event);

  void set_tree_focus(Element element, bool focused);
  void focus_next(bool reverse);

  Window window;
  Tree tree;
  bool debug_mode_ = false;

  Resources res;

  HorizontalLayoutSystem horizontal_layout_system;
  VerticalLayoutSystem vertical_layout_system;
  TextBoxSystem text_box_system;
  TextInputSystem text_input_system;
  ButtonSystem button_system;
  DropdownSystem dropdown_system;
  FloatingSystem floating_system;
  CheckboxSystem checkbox_system;
  ElementSystems systems;

  Element element_focus;
  Element element_hover;
  int next_float_priority = 0;
};

} // namespace datagui
