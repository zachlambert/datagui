#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/tree/tree.hpp"
#include "datagui/visual/window.hpp"

#include "datagui/visual/font.hpp"
#include "datagui/visual/geometry_renderer.hpp"
#include "datagui/visual/text_renderer.hpp"

#include "datagui/element/linear_layout.hpp"
#include "datagui/element/text.hpp"

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  bool running() const;
  void render_begin();
  void render_end();

  bool linear_layout(
      const std::function<void(LinearLayoutStyle&)>& set_style = nullptr);
  void container_end();

  void text(
      const std::string& text,
      const std::function<void(TextStyle&)>& set_style = nullptr);

#if 0
  DataPtr<std::string> text_input(const std::string& initial_text);
#endif

private:
  ElementSystem& element_system(ElementType type);

  Window window;
  Tree tree;

  FontManager font_manager;
  GeometryRenderer geometry_renderer;
  TextRenderer text_renderer;

  LinearLayoutSystem linear_layout_system;
  TextSystem text_system;
};

} // namespace datagui
