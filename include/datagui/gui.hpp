#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/datapack/reader.hpp"
#include "datagui/datapack/writer.hpp"
#include "datagui/element/series.hpp"
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

  template <typename T>
  requires datapack::writeable<T> && datapack::readable<T>
  Variable<T> edit_variable(const T& initial_value = T()) {
    {
      auto element = tree.next();
      if (element->system == -1) {
        DATAGUI_LOG("Gui::edit_variable", "Construct new Series");
        element->props = UniqueAny::Make<SeriesProps>();
        element->system = systems.find<SeriesSystem>();
      }
      auto& props = *element->props.cast<SeriesProps>();
      props.set_style(*sm);
    }

    DATAGUI_LOG("Gui::edit_variable", "DOWN (1)");
    tree.down();

    auto var = variable<T>(initial_value);

    {
      auto element = tree.next();
      if (element->system == -1) {
        DATAGUI_LOG("Gui::edit_variable", "Construct new Series");
        element->props = UniqueAny::Make<SeriesProps>();
        element->system = systems.find<SeriesSystem>();
      }
      auto& props = *element->props.cast<SeriesProps>();
      props.set_style(*sm);
    }

    if (!tree.down_if()) {
      DATAGUI_LOG("Gui::edit_variable", "UP (1) (no revisit)");
      tree.up();
      return var;
    }
    DATAGUI_LOG("Gui::edit_variable", "DOWN (2) (revisit)");

    if (var.modified()) {
      DATAGUI_LOG("Gui::edit_variable", "Write variable -> GUI");
      GuiWriter(systems, tree, sm).value(*var);
    } else {
      DATAGUI_LOG("Gui::edit_variable", "Read GUI -> variable");
      T new_value;
      GuiReader(systems, tree).value(new_value);
      var.set(new_value);
    }

    DATAGUI_LOG("Gui::edit_variable", "UP (2) (after revisit)");
    tree.up();
    DATAGUI_LOG("Gui::edit_variable", "UP (1) (after revisit)");
    tree.up();

    return var;
  }

  void style(const Style& style) {
    sm->push(style);
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
  ElementSystemList systems;

  ElementPtr element_focus;
  ElementPtr element_hover;
  int next_float_priority = 0;
};

} // namespace datagui
