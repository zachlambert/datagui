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
#include "datagui/element/series.hpp"
#include "datagui/element/text_box.hpp"
#include "datagui/element/text_input.hpp"
#include "datagui/log.hpp"
#include "datapack/datapack.hpp"

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  bool running() const;
  void begin();
  void end();

  // Series

  bool series_begin(const Style& style = Style());
  void series_begin_force(const Style& style = Style());
  void series_end();

  // Text box

  void text_box(const std::string& text, const Style& style = Style());

  // Button

  bool button(const std::string& text, const Style& style = Style());

  // Text input

  const std::string* text_input(
      const std::string& initial_text = "",
      const Style& style = Style());
  void text_input(
      const Variable<std::string>& text,
      const Style& style = Style());

  void text_input_write(const std::string& value, const Style& style = Style());

  const std::string& text_input_read();

  // Checkbox

  const bool* checkbox(
      const bool& initial_checked = false,
      const Style& style = Style());

  void checkbox(const Variable<bool>& checked, const Style& style = Style());

  void checkbox_write(bool value, const Style& style = Style());

  bool checkbox_read();

  // Dropdown

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

  // Variable

  template <typename T>
  Variable<T> variable(const T& initial_value = T()) {
    // Capture initial_value by value
    return tree.variable<T>([initial_value]() { return initial_value; });
  }

  // Datapack

  template <typename T>
  requires datapack::writeable<T> && datapack::readable<T>
  Variable<T> edit_variable(const T& initial_value = T());

  // Style

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

  SeriesSystem series_system;
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

class GuiWriter : public datapack::Writer {
public:
  GuiWriter(Gui& gui) : gui(gui) {}

  void number(datapack::NumberType type, const void* value) override;
  void boolean(bool value) override;
  void string(const char*) override;
  void enumerate(int value, const std::span<const char*>& labels) override;
  void binary(const std::span<const std::uint8_t>& data) override;

  void optional_begin(bool has_value) override;
  void optional_end() override;

  void variant_begin(int value, const std::span<const char*>& labels) override;
  void variant_end() override;

  void object_begin() override;
  void object_next(const char* key) override;
  void object_end() override;

  void tuple_begin() override;
  void tuple_next() override;
  void tuple_end() override;

  void list_begin() override;
  void list_next() override;
  void list_end() override;

private:
  Gui& gui;
  std::stack<Variable<std::size_t>> list_sizes;
};

class GuiReader : public datapack::Reader {
public:
  GuiReader(Gui& gui) : gui(gui) {}

  void number(datapack::NumberType type, void* value) override;
  bool boolean() override;
  const char* string() override;
  int enumerate(const std::span<const char*>& labels) override;
  std::span<const std::uint8_t> binary() override;

  bool optional_begin() override;
  void optional_end() override;

  int variant_begin(const std::span<const char*>& labels) override;
  void variant_end() override;

  void object_begin() override;
  void object_next(const char* key) override;
  void object_end() override;

  void tuple_begin() override;
  void tuple_next() override;
  void tuple_end() override;

  void list_begin() override;
  bool list_next() override;
  void list_end() override;

private:
  Gui& gui;
  std::stack<std::pair<Variable<std::size_t>, std::size_t>> list_sizes;
};

template <typename T>
requires datapack::writeable<T> && datapack::readable<T>
Variable<T> Gui::edit_variable(const T& initial_value) {
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

} // namespace datagui
