#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/datapack/edit.hpp"
#include "datagui/datapack/reader.hpp"
#include "datagui/element/args.hpp"
#include "datagui/element/tree.hpp"
#include "datagui/system/system.hpp"
#include "datagui/theme.hpp"
#include "datagui/visual/renderer.hpp"
#include "datagui/visual/window.hpp"
#include <memory>
#include <set>
#include <vector>

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  // Setup

  bool running() const;
  void poll();

  // Common end method

  void end();

  // Key and dependencies

  void key(std::size_t key) {
    next_key = key;
  }
  template <typename T>
  void key(const T& key) {
    this->key(std::hash<T>{}(key));
  }

  template <typename T>
  void depend_variable(const Var<T>& var) {
    if (stack.empty()) {
      return;
    }
    stack.top().first.add_variable_dep(var);
  }
  void depend_condition(const std::function<bool()>& condition) {
    if (stack.empty()) {
      return;
    }
    stack.top().first.add_condition_dependency(condition);
  }
  void depend_timeout(double period) {
    if (stack.empty()) {
      return;
    }
    stack.top().first.add_timeout_dependency(period);
  }

  // Elements

  void button(
      const std::string& text,
      const std::function<void()>& callback = {});

  void checkbox(
      bool initial_value,
      const std::function<void(bool)>& callback = {});
  void checkbox(bool& value) {
    checkbox(value, [&value](bool new_value) { value = new_value; });
  }
  void checkbox(const Var<bool>& value);

  void dropdown(
      const std::vector<std::string>& choices,
      int initial_choice,
      const std::function<void(int)>& callback = {});
  void dropdown(const std::vector<std::string>& choices, int& choice) {
    dropdown(choices, choice, [&choice](int new_choice) {
      choice = new_choice;
    });
  }
  void dropdown(
      const std::vector<std::string>& choices,
      const Var<int>& choice);

  bool floating(
      const Var<bool>& open,
      const std::string& title,
      float width,
      float height);

  bool labelled(const std::string& label);

  bool section(const std::string& label);

  bool series();

  void text_input(
      const std::string& initial_value,
      const std::function<void(const std::string& callback)>& callback = {});
  void text_input(std::string& value) {
    text_input(value, [&value](const std::string& new_value) {
      value = new_value;
    });
  }
  void text_input(const Var<std::string>& value);

  void text_box(const std::string& text);

  template <typename T>
  Var<T> variable(const T& initial_value = T()) {
    if (!var_current.valid()) {
      if (stack.empty()) {
        var_current = tree.var();
      } else {
        var_current = stack.top().first.var();
      }
    }
    if (!var_current) {
      Var<T> result = var_current.create<T>(initial_value);
      var_current = var_current.next();
      return result;
    } else {
      Var<T> result = var_current.as<T>();
      var_current = var_current.next();
      return result;
    }
  }

  template <typename T>
  Var<T> variable(const std::function<T()>& construct) {
    if (!var_current.valid()) {
      if (stack.empty()) {
        var_current = tree.var();
      } else {
        var_current = stack.top().first.var();
      }
    }
    if (!var_current) {
      Var<T> result = var_current.create<T>(construct());
      var_current = var_current.next();
      return result;
    } else {
      Var<T> result = var_current.as<T>();
      var_current = var_current.next();
      return result;
    }
  }

#if 0
  template <typename T>
  const T* edit() {
    if (!series_begin()) {
      return nullptr;
    }
    auto value = variable<T>([] { return T(); });
    auto schema = variable<datapack::Schema>(
        []() { return datapack::Schema::make<T>(); });
    ConstElementPtr node = current;
    bool changed = datapack_edit(*this, *schema);
    series_end();

    if (changed) {
      value.set(datapack_read<T>(node));
      return &(*value);
    }
    return nullptr;
  }
#endif

#if 0
  template <typename T>
  void edit(const Var<T>& var) {
    auto var_version = variable<int>(0);
    auto schema = variable<datapack::Schema>(
        []() { return datapack::Schema::make<T>(); });

    if (var.version() != *value_version) {
      datapack_write(*this, *var);
    } else {
      datapack_edit(*this, *schema);
      datapack_read(current.prev(), var.mut());
      var_version
    }
  }
#endif

  SeriesArgs& args_series() {
    return args_series_;
  }

  FloatingArgs& args_floating() {
    return args_floating_;
  }

private:
  void check_begin();
  void render();
#ifdef DATAGUI_DEBUG
  void debug_render();
#endif
  void calculate_sizes();

  void event_handling();
  ElementPtr get_leaf_node(const Vecf& position);
  void event_handling_left_click(const MouseEvent& event);
  void event_handling_hover(const Vecf& mouse_pos);
  void event_handling_scroll(const ScrollEvent& event);
  void trigger_handler(ConstElementPtr element);

  void set_tree_focus(ElementPtr element, bool focused);
  void focus_next(bool reverse);

  Window window;
  Tree tree;

#ifdef DATAGUI_DEBUG
  bool debug_mode_ = false;
#endif

  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
  Renderer renderer;
  std::vector<std::unique_ptr<System>> systems;

  std::stack<std::pair<ElementPtr, VarPtr>> stack;
  ElementPtr current;
  VarPtr var_current;

  ElementPtr element_focus;
  ElementPtr element_hover;
  int next_float_priority = 0;

  std::size_t read_key() {
    std::size_t key = next_key;
    next_key = 0;
    return key;
  }
  std::size_t next_key = 0;

  struct Compare {
    bool operator()(const ElementPtr& lhs, const ElementPtr& rhs) const {
      return lhs.state().float_priority <= rhs.state().float_priority;
    }
  };
  std::set<ElementPtr, Compare> floating_elements;

  SeriesArgs args_series_;
  FloatingArgs args_floating_;

  // For convenience
  System& system(ConstElementPtr element) {
    return *systems[static_cast<std::size_t>(element.type())];
  }
  void set_input_state(ElementPtr element) {
    system(element).set_input_state(element);
  }
  void set_dependent_state(ElementPtr element) {
    system(element).set_dependent_state(element);
  };
  void render(ConstElementPtr element) {
    system(element).render(element, renderer);
  }
  bool mouse_event(ElementPtr element, const MouseEvent& event) {
    return system(element).mouse_event(element, event);
  }
  bool mouse_hover(ElementPtr element, const Vecf& mouse_pos) {
    return system(element).mouse_hover(element, mouse_pos);
  }
  bool scroll_event(ElementPtr element, const ScrollEvent& event) {
    return system(element).scroll_event(element, event);
  }
  bool key_event(ElementPtr element, const KeyEvent& event) {
    return system(element).key_event(element, event);
  }
  bool text_event(ElementPtr element, const TextEvent& event) {
    return system(element).text_event(element, event);
  }
  void focus_enter(ElementPtr element) {
    system(element).focus_enter(element);
  }
  bool focus_leave(ElementPtr element, bool success) {
    return system(element).focus_leave(element, success);
  }
};

} // namespace datagui
