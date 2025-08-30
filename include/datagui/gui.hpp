#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#if 0
#include "datagui/gui_datapack.hpp"
#endif
#include "datagui/tree/tree.hpp"
#include "datagui/visual/renderer.hpp"
#include "datagui/visual/window.hpp"
#include <memory>
#include <vector>

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  bool running() const;
  bool begin();
  void end();
  void poll();

  bool series_begin();
  void series_end();

  void text_box(const std::string& text);

  bool button(const std::string& text);

  const std::string* text_input(const std::string& initial_value);
  void text_input(const Variable<std::string>& value);

  const bool* checkbox(bool initial_value = false);
  void checkbox(const Variable<bool>& value);

  const int* dropdown(
      const std::vector<std::string>& choices,
      int initial_choice = -1);
  void dropdown(
      const std::vector<std::string>& choices,
      const Variable<int>& choice);

  bool floating_begin(const Variable<bool>& open, const std::string& title);
  void floating_end();

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
    sm->push(style);
  }

  template <typename System>
  requires std::is_base_of_v<ElementSystem, System>
  int find_system() const {
    for (int i = 0; i < systems.size(); i++) {
      if (dynamic_cast<const System*>(systems[i].get())) {
        return i;
      }
    }
    return -1;
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
#ifdef DATAGUI_DEBUG
  bool debug_mode_ = false;
#endif

  std::shared_ptr<FontManager> fm;
  std::shared_ptr<StyleManager> sm;
  Renderer renderer;
  std::vector<std::unique_ptr<ElementSystem>> systems;

  ElementPtr element_focus;
  ElementPtr element_hover;
  int next_float_priority = 0;
};

} // namespace datagui
