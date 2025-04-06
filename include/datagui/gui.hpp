#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/tree/tree.hpp"
#include "datagui/visual/window.hpp"

#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/horizontal_layout.hpp"
#include "datagui/element/text_box.hpp"
#include "datagui/element/text_input.hpp"
#include "datagui/element/vertical_layout.hpp"

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  void set_debug_mode(bool value) {
    debug_mode_ = value;
  }
  bool running() const;
  void begin();
  void end();

  bool horizontal_layout(
      const std::function<void(HorizontalLayoutStyle&)>& set_style = nullptr);
  bool vertical_layout(
      const std::function<void(VerticalLayoutStyle&)>& set_style = nullptr);
  void layout_end();

  void text_box(
      const std::string& text,
      const std::function<void(TextBoxStyle&)>& set_style = nullptr);

  Tree::ConstData<std::string> text_input(
      const std::function<void(TextInputStyle&)>& set_style = nullptr);

  bool button(
      const std::string& text,
      const std::function<void(ButtonStyle&)>& set_style = nullptr);

  template <typename T>
  Tree::Data<T> data(T initial_value) {
    return tree.data_parent<T>([&]() { return initial_value; });
  }

private:
  void render();
  void calculate_sizes();

  void event_handling();
  void event_handling_left_click(const MouseEvent& event);

  void set_tree_focus(Tree::Ptr node, bool value);
  void focus_next(bool reverse);

  void deinit_node(Tree::ConstPtr node);
  ElementSystem& element_system(ElementType type);

  Window window;
  Tree tree;
  bool debug_mode_ = false;

  FontManager font_manager;
  GeometryRenderer geometry_renderer;
  TextRenderer text_renderer;

  HorizontalLayoutSystem horizontal_layout_system;
  VerticalLayoutSystem vertical_layout_system;
  TextBoxSystem text_box_system;
  TextInputSystem text_input_system;
  ButtonSystem button_system;

  Tree::Ptr node_focus;
};

} // namespace datagui
