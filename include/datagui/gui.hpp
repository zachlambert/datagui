#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/datapack/reader.hpp"
#include "datagui/datapack/writer.hpp"
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

namespace dgui {

class Plotter;
class Canvas2d;
class Canvas3d;

class Gui {
public:
  Gui(const std::string& title = "datagui",
      std::size_t width = 900,
      std::size_t height = 600);

  // Setup

  bool poll();

  // Common end method

  void end();

  // DGUI_SCOPE(gui) will create a DeferEnd() variable
  // to automatically call gui.end() when exiting the scope
  class DeferEnd {
  public:
    ~DeferEnd() {
      if (gui) {
        gui->end();
        gui = nullptr;
      }
    }
    DeferEnd(DeferEnd&& other) : gui(other.gui) {
      other.gui = nullptr;
    }
    DeferEnd(const DeferEnd&) = delete;
    DeferEnd& operator=(const DeferEnd&) = delete;
    DeferEnd& operator=(DeferEnd&&) = delete;

  private:
    DeferEnd(Gui* gui) : gui(gui) {}
    Gui* gui;
    friend class Gui;
  };
  [[nodiscard]] DeferEnd defer_end() {
    return DeferEnd(this);
  }

  // Key and dependencies

  void key(std::size_t key) {
    next_key = key;
  }
  template <typename T>
  void key(const T& key) {
    this->key(std::hash<T>{}(key));
  }

  // Elements

  [[nodiscard]] bool button(const std::string& text);

  [[nodiscard]] std::optional<bool> checkbox(bool initial_value);
  bool checkbox_v(bool& value);

  [[nodiscard]] bool collapsable(const std::string& label);

  [[nodiscard]] std::optional<Color> color_picker(const Color& initial_value);
  bool color_picker_v(Color& value);

  [[nodiscard]] bool dropdown(const std::string& label);

  void group();

  [[nodiscard]] bool popup(
      bool& open,
      const std::string& title,
      float width,
      float height);

  [[nodiscard]] std::optional<int> select(
      int initial_choice,
      const std::vector<std::string>& choices);
  bool select_v(int& choice, const std::vector<std::string>& choices);

  template <typename T>
  [[nodiscard]] std::optional<T> slider(T initial_value, T lower, T upper);
  template <typename T>
  bool slider_v(T& value, T lower, T upper);

  void hsplit(float ratio);
  void vsplit(float ratio);

  void tabs(size_t initial_tab = 0);
  [[nodiscard]] bool tab_group(const std::string& label);

  [[nodiscard]] const std::string* text_input(const std::string& initial_value);
  bool text_input_v(std::string& value);

  template <typename T>
  [[nodiscard]] std::optional<T> number_input(T initial_value);
  template <typename T>
  bool number_input_v(T& value);

  void text_box(const std::string& text);

  template <typename T>
  T& variable(const T& initial_value = T()) {
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
      return *result;
    } else {
      Var<T> result = var_current.as<T>();
      var_current = var_current.next();
      return *result;
    }
  }

  template <typename T>
  T& variable(const std::function<T()>& construct) {
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
      return *result;
    } else {
      Var<T> result = var_current.as<T>();
      var_current = var_current.next();
      return *result;
    }
  }

  template <dpack::serializable T>
  const T* edit(const std::string& label, const T& initial_value = T()) {
    bool is_new = current.expect(Type::Group, read_key());
    current.group().layout.tight = true;
    move_down();

    if (edit_skip()) {
      return nullptr;
    }

    T& var = variable<T>(initial_value);
    if (is_new) {
      edit_write(var, label);
      current = current.next();
      end();
      return nullptr;
    }
    bool changed = edit_read(var, label);
    current = current.next();
    end();

    if (changed) {
      return &var;
    }
    return nullptr;
  }

  template <dpack::serializable T>
  bool edit_v(const std::string& label, T& value) {
    bool is_new = current.expect(Type::Group, read_key());
    current.group().layout.tight = true;
    move_down();

    if (edit_skip()) {
      return false;
    }

    bool has_changed = GuiReader::peek_changed(current);
    if (is_new || !has_changed) {
      edit_write(value, label);
      current = current.next();
      end();
      return false;
    }
    edit_read(value, label);
    current = current.next();
    end();
    return true;
  }

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
  template <dpack::serializable T>
  bool edit_read(T& value, const std::string& label) {
    GuiReader reader(current, label);
    reader.value(value);
    return reader.changed();
  }
  template <dpack::serializable T>
  void edit_write(const T& value, const std::string& label) {
    GuiWriter writer(current, label);
    writer.value(value);
  }
  bool edit_skip() {
    bool can_skip = current && current.type() == Type::Collapsable &&
                    !current.collapsable().open;
    if (can_skip) {
      current = current.next();
      end();
    }
    return can_skip;
  }

  void move_down();

  void render();
#ifdef DGUI_DEBUG
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

#ifdef DGUI_DEBUG
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

#define DGUI_SCOPE(gui_name) auto defer_end = gui_name.defer_end()

} // namespace dgui
