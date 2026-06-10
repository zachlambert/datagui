#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/datapack/edit.hpp"
#include "datagui/element/args.hpp"
#include "datagui/element/system.hpp"
#include "datagui/element/tree.hpp"
#include "datagui/theme.hpp"
#include "datagui/viewport/canvas2d.hpp"
#include "datagui/viewport/canvas3d.hpp"
#include "datagui/viewport/plotter.hpp"
#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/gui_renderer.hpp"
#include "datagui/visual/window.hpp"
#include <memory>
#include <optional>
#include <set>
#include <unordered_set>
#include <vector>

namespace datagui {

class Plotter;
class Canvas2d;
class Canvas3d;

class Gui {
public:
  Gui(const std::string& title = "datagui",
      std::size_t width = 900,
      std::size_t height = 600);

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

  // Elements

  bool button(const std::string& text);

  std::optional<bool> checkbox(bool initial_value);
  bool checkbox_v(bool& value);

  bool collapsable(const std::string& label);

  std::optional<Color> color_picker(const Color& initial_value);
  bool color_picker_v(Color& value);

  bool dropdown(const std::string& label);

  void group();

  bool popup(bool& open, const std::string& title, float width, float height);

  std::optional<int> select(
      int initial_choice,
      const std::vector<std::string>& choices);
  bool select_v(int& choice, const std::vector<std::string>& choices);

  template <typename T>
  std::optional<T> slider(T initial_value, T lower, T upper);
  template <typename T>
  bool slider_v(T& value, T lower, T upper);

  void hsplit(float ratio);
  void vsplit(float ratio);

#if 0
  void tabs(std::size_t initial_tab, const std::vector<std::string>& labels);
#endif

  const std::string* text_input(const std::string& initial_value);
  bool text_input_v(std::string& value);

  template <typename T>
  std::optional<T> number_input(T initial_value);
  template <typename T>
  bool number_input_v(T& value);

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
  requires std::is_default_constructible_v<T>
  void edit(
      const std::string& label,
      const std::function<void(const T&)>& callback,
      const T& initial_value = T()) {
    bool is_new = !current;
    args_.tight();
    if (!group()) {
      return;
    }
    auto schema = variable<dpack::Schema>([]() {
      return dpack::Schema::make<T>();
    });
    if (is_new) {
      datapack_write(*this, label, initial_value);
    } else {
      datapack_edit(*this, label, *schema);
      auto node_capture = current.prev();
      T result;
      datapack_read<T>(node_capture, result);
      callback(result);
    }
    end();
  }

  template <typename T>
  void edit(const std::string& label, const Var<T>& var) {
    bool is_new = !current;
    args_.tight();
    if (!group()) {
      return;
    }

    auto var_version = variable<int>(0);
    auto schema = variable<dpack::Schema>([]() {
      return dpack::Schema::make<T>();
    });

    if (var.version() != var_version.mut_internal() || is_new) {
      var_version.mut_internal() = var.version();
      overwrite = true;
      datapack_write(*this, label, *var);
      overwrite = false;
    } else {
      datapack_edit(*this, label, *schema);
      if (!is_new) {
        auto node_capture = current.prev();
        datapack_read<T>(node_capture, var.mut());
      }
    }
    end();
  }

  template <typename T>
  requires dpack::supported<T>
  void edit(const std::string& label, T& value) {
    bool is_new = !current;
    args_.tight();
    if (!group()) {
      return;
    }
    auto schema = variable<dpack::Schema>([&]() {
      return dpack::Schema::make<T>();
    });
    if (is_new) {
      datapack_write(*this, label, value);
    } else {
      datapack_edit(*this, label, *schema);
      auto node_capture = current.prev();
      datapack_read<T>(node_capture, value);
    }
    end();
  }
#endif

  Args& args() {
    return args_;
  }

  Canvas2d& canvas2d(float width, float height) {
    return viewport<Canvas2d>(width, height);
  }
  Canvas3d& canvas3d(float width, float height) {
    return viewport<Canvas3d>(width, height);
  }
  Plotter& plotter(float width, float height) {
    return viewport<Plotter>(width, height);
  }

private:
  void check_begin();
  void move_down();

  void render();
#ifdef DATAGUI_DEBUG
  void debug_render();
#endif
  void calculate_sizes();

  void event_handling();
  ElementPtr get_leaf_node(const Vec2& position);
  void event_handling_left_click(const MouseEvent& event);
  void event_handling_right_click(const MouseEvent& event);
  void event_handling_middle_click(const MouseEvent& event);
  void event_handling_hover(const Vec2& mouse_pos);
  void event_handling_scroll(const ScrollEvent& event);

  void change_tree_focus(ElementPtr from, ElementPtr to);
  void focus_next(bool reverse);

  template <typename T>
  requires std::is_base_of_v<Viewport, T>
  T& viewport(float width, float height);

  Window window;
  Tree tree;

#ifdef DATAGUI_DEBUG
  bool debug_mode_ = false;
#endif

  std::shared_ptr<FontManager> fm;
  std::shared_ptr<Theme> theme;
  GuiRenderer renderer;
  std::vector<std::unique_ptr<System>> systems;

  std::stack<std::pair<ElementPtr, VarPtr>> stack;
  ElementPtr current;
  VarPtr var_current;

  ElementPtr element_focus;
  ElementPtr element_hover;
  ElementPtr element_left_held;
  ElementPtr element_middle_held;
  int next_float_priority = 0;
  std::vector<std::function<void()>> misc_events;

  std::size_t read_key() {
    std::size_t key = next_key;
    next_key = 0;
    return key;
  }
  std::size_t next_key = 0;
  bool overwrite = false; // Only used for datapack_write, special case

  std::unordered_set<ElementPtr, ElementPtr::HashFunc> floating_elements;
  std::set<ElementPtr, ElementPtr::FloatCompare> ordered_floating_elements;

  Args args_;

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
  void mouse_event(ElementPtr element, const MouseEvent& event) {
    system(element).mouse_event(element, event);
  }
  void mouse_hover(ElementPtr element, const Vec2& mouse_pos) {
    system(element).mouse_hover(element, mouse_pos);
  }
  bool scroll_event(ElementPtr element, const ScrollEvent& event) {
    return system(element).scroll_event(element, event);
  }
  void key_event(ElementPtr element, const KeyEvent& event) {
    system(element).key_event(element, event);
  }
  void text_event(ElementPtr element, const TextEvent& event) {
    system(element).text_event(element, event);
  }
  void focus_enter(ElementPtr element) {
    system(element).focus_enter(element);
  }
  void focus_leave(ElementPtr element, bool success) {
    return system(element).focus_leave(element, success);
  }
  void focus_tree_leave(ElementPtr element) {
    return system(element).focus_tree_leave(element);
  }
};

} // namespace datagui
