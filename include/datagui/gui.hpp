#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/datapack/edit.hpp"
#include "datagui/datapack/reader.hpp"
#include "datagui/datapack/writer.hpp"
#include "datagui/element/args.hpp"
#include "datagui/element/system.hpp"
#include "datagui/element/tree.hpp"
#include "datagui/theme.hpp"
#include "datagui/visual/renderer.hpp"
#include "datagui/visual/window.hpp"
#include <memory>
#include <set>
#include <unordered_set>
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

  // Elements

  void button(const std::string& text, const std::function<void()>& callback);

  void checkbox(bool initial_value, const std::function<void(bool)>& callback);
  void checkbox(bool& value) {
    checkbox(value, [&value](bool new_value) { value = new_value; });
  }
  void checkbox(const Var<bool>& var);

  bool collapsable(const std::string& label);

  void color_picker(
      const Color& initial_value,
      const std::function<void(const Color&)>& callback);
  void color_picker(const Var<Color>& var);
  void color_picker(Color& value) {
    color_picker(value, [&value](const Color& new_value) {
      value = new_value;
    });
  }

  bool dropdown(const std::string& label);

  bool group();

  bool popup(
      const Var<bool>& open,
      const std::string& title,
      float width,
      float height);

  void select(
      const std::vector<std::string>& choices,
      int initial_choice,
      const std::function<void(int)>& callback);
  void select(const std::vector<std::string>& choices, int& choice) {
    select(choices, choice, [&choice](int new_choice) { choice = new_choice; });
  }
  void select(const std::vector<std::string>& choices, const Var<int>& choice);

  template <typename T>
  void slider(
      T initial_value,
      T lower,
      T upper,
      const std::function<void(T)>& callback);
  template <typename T>
  void slider(T lower, T upper, const Var<T>& var);
  template <typename T>
  void slider(T& value, T lower, T upper) {
    slider(value, lower, upper, [&value](T new_value) { value = new_value; });
  }

  bool hsplit(float ratio);
  bool vsplit(float ratio);

  bool tabs(
      const std::vector<std::string>& labels,
      std::size_t initial_tab = 0);

  void text_input(
      const std::string& initial_value,
      const std::function<void(const std::string& callback)>& callback);
  void text_input(const Var<std::string>& var);
  void text_input(std::string& value) {
    text_input(value, [&value](const std::string& new_value) {
      value = new_value;
    });
  }

  template <typename T>
  void number_input(
      const T& initial_value,
      const std::function<void(T)>& callback);
  template <typename T>
  void number_input(const Var<T>& var);
  template <typename T>
  void number_input(T& value) {
    number_input(value, [&value](T new_value) { value = new_value; });
  }

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
      if (overwrite) {
        result.mut_internal() = initial_value;
      }
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
  void edit(const std::function<void(const T&)>& callback) {
    bool is_new = !current;
    args_.tight();
    if (!group()) {
      return;
    }
    auto schema = variable<datapack::Schema>(
        []() { return datapack::Schema::make<T>(); });
    datapack_edit(*this, *schema);
    if (!is_new) {
      auto node_capture = current.prev();
      callback(datapack_read<T>(node_capture));
    }
    end();
  }

  template <typename T>
  void edit(const Var<T>& var) {
    bool is_new = !current;
    args_.tight();
    if (!group()) {
      return;
    }

    auto var_version = variable<int>(0);
    auto schema = variable<datapack::Schema>(
        []() { return datapack::Schema::make<T>(); });

    if (var.version() != var_version.mut_internal() || is_new) {
      var_version.mut_internal() = var.version();
      overwrite = true;
      datapack_write(*this, *var);
      overwrite = false;
    } else {
      datapack_edit(*this, *schema);
      if (!is_new) {
        auto node_capture = current.prev();
        var.set(datapack_read<T>(node_capture));
      }
    }
    end();
  }

  Args& args() {
    return args_;
  }

  template <typename T>
  requires std::is_base_of_v<Viewport, T>
  T* viewport(float width, float height) {
    check_begin();
    current.expect(Type::ViewportPtr, read_key());
    auto& viewport = current.viewport();
    if (!viewport.viewport) {
      viewport.viewport = std::make_unique<T>();
      viewport.viewport->init(width, height, fm);
    }
    // Renderered width/height can differ to the initial width/height
    // above - this defines the size used for the framebuffer
    viewport.width = width;
    viewport.height = height;

    if (!current.dirty()) {
      current = current.next();
      return nullptr;
    }
    move_down();
    viewport.viewport->begin();
    T* ptr = dynamic_cast<T*>(viewport.viewport.get());
    assert(ptr);
    return ptr;
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
  void event_handling_hover(const Vec2& mouse_pos);
  void event_handling_scroll(const ScrollEvent& event);

  void change_tree_focus(ElementPtr from, ElementPtr to);
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
