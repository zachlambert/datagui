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

  bool running() const;
  bool begin();
  void end();
  void poll();

  void key(std::size_t key) {
    next_key = key;
  }
  template <typename T>
  void key(const T& key) {
    key(std::hash<T>{}());
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

  bool series_begin();
  void series_end();

  bool labelled_begin(const std::string& label);
  void labelled_end();

  bool section_begin(const std::string& label);
  void section_end();

  void text_box(const std::string& text);

  bool button(const std::string& text);

  const std::string* text_input(const std::string& initial_value);
  void text_input(const Var<std::string>& value);

  const bool* checkbox(bool initial_value = false);
  void checkbox(const Var<bool>& value);

  const int* dropdown(
      const std::vector<std::string>& choices,
      int initial_choice = -1);
  void dropdown(
      const std::vector<std::string>& choices,
      const Var<int>& choice);

  bool floating_begin(
      const Var<bool>& open,
      const std::string& title,
      float width,
      float height);
  void floating_end();

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
  System& system(ConstElementPtr element) {
    return *systems[static_cast<std::size_t>(element.type())];
  }

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
};

} // namespace datagui
