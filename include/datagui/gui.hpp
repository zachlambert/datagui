#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#if 0
#include "datagui/gui_datapack.hpp"
#endif
#include "datagui/tree/tree.hpp"
#include "datagui/visual/window.hpp"

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  bool running() const;
  void begin();
  void end();

  bool series_begin();
  void series_end();

  void text_box(const std::string& text);

#if 0
  bool series_begin(const Style& style = Style());
  void series_begin_force(const Style& style = Style());
  void series_end();

  void text_box(const std::string& text, const Style& style = Style());

  bool button(const std::string& text, const Style& style = Style());

  const std::string* text_input(
      const std::string& initial_text = "",
      const Style& style = Style());
  void text_input(
      const Variable<std::string>& text,
      const Style& style = Style());
  void text_input_write(const std::string& value, const Style& style = Style());
  const std::string& text_input_read();

  const bool* checkbox(
      const bool& initial_checked = false,
      const Style& style = Style());
  void checkbox(const Variable<bool>& checked, const Style& style = Style());
  void checkbox_write(bool value, const Style& style = Style());
  bool checkbox_read();

  const int* dropdown(
      const std::vector<std::string>& choices,
      int initial_choice = -1,
      const Style& style = Style());
  void dropdown(
      const std::vector<std::string>& choices,
      const Variable<int>& choice,
      const Style& style = Style());
  void dropdown_write(
      const std::vector<std::string>& choices,
      int choice,
      const Style& style = Style());
  int dropdown_read();

  // Floating

  bool floating_begin(
      const Variable<bool>& open,
      const std::string& title,
      const Style& style = Style());
  void floating_end();

#endif

  template <typename T>
  Variable<T> variable(const T& initial_value = T()) {
    // Capture initial_value by value
    return tree.variable<T>([initial_value]() { return initial_value; });
  }

#if 0

  template <typename T>
  requires datapack::writeable<T> && datapack::readable<T>
  Variable<T> edit_variable(const T& initial_value = T()) {
    series_begin_force();
    auto var = variable<T>(initial_value);
    if (var.modified() || var.is_new()) {
      if (var.modified()) {
        DATAGUI_LOG("[Gui::edit_variable] Variable modified -> update GUI");
      } else {
        DATAGUI_LOG("[Gui::edit_variable] Variable is new -> update GUI");
      }
      series_begin_force();
      GuiWriter(*this).value(*var);
      series_end();
    } else {
      if (series_begin()) {
        DATAGUI_LOG("[Gui::edit_variable] Variable revisit -> update VAR");
        T new_value;
        GuiReader(*this).value(new_value);
        var.set(new_value);
        series_end();
      }
    }
    series_end();
    return var;
  }
#endif

  // Style

  void style(const Style& style) {
    res.style_manager.push(style);
  }

private:
  void render();
  void calculate_sizes();

  void event_handling();
  ElementPtr get_leaf_node(const Vecf& position);
  void event_handling_left_click(const MouseEvent& event);
  void event_handling_hover(const Vecf& mouse_pos);
  void event_handling_scroll(const ScrollEvent& event);

  void set_tree_focus(ElementPtr element, bool focused);
  void focus_next(bool reverse);

  Window window;
  Tree tree;
  bool debug_mode_ = false;

  Resources res;
  ElementPtr element_focus;
  ElementPtr element_hover;
  int next_float_priority = 0;
};

} // namespace datagui
