#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/datapack/reader.hpp"
#include "datagui/datapack/writer.hpp"
#include "datagui/element/args.hpp"
#include "datagui/element/system.hpp"
#include "datagui/element/tree.hpp"
#include "datagui/render/executor.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/render/program_registry.hpp"
#include "datagui/render/state/draw_list.hpp"
#include "datagui/render/window.hpp"
#include "datagui/theme.hpp"
#include "datagui/viewport/viewport.hpp"
#include "datagui/viewport/canvas2d.hpp"
#include "datagui/viewport/canvas3d.hpp"
#include "datagui/viewport/plotter.hpp"
#include "datagui/element/layer_manager.hpp"
#include "datagui/viewport/viewport.hpp"
#include <memory>
#include <optional>
#include <vector>

namespace dgui {

class Plotter;
class Canvas2d;
class Canvas3d;
class PopupSystem;

class Gui {
public:
  Gui();
  ~Gui();

  // Setup

  void open(
      const std::string& title = "datagui",
      std::size_t width = 900,
      std::size_t height = 600);
  void close();

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

  Canvas2d& canvas2d();
  Canvas3d& canvas3d();
  Plotter& plotter();

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
  T& viewport();

  Window window;
  ProgramRegistry program_registry;
  Tree tree;
  Executor executor;

#ifdef DGUI_DEBUG
  bool debug_mode_ = false;
#endif

  std::shared_ptr<FontRegistry> font_registry;
  std::shared_ptr<Theme> theme;
  DrawList dl;

  std::stack<std::pair<ElementPtr, VarPtr>> stack;
  ElementPtr current;
  VarPtr var_current;

  ElementPtr element_focus;
  ElementPtr element_hover;
  ElementPtr element_left_held;
  ElementPtr element_middle_held;
  ElementPtr element_focus_defer;
  int focus_index = 0;

  std::size_t read_key() {
    std::size_t key = next_key;
    next_key = 0;
    return key;
  }
  std::size_t next_key = 0;

  Args args_;
  SystemSet systems;
  LayerManager layer_manager;
};

#define DGUI_SCOPE(gui_name) auto defer_end = gui_name.defer_end()

} // namespace dgui
