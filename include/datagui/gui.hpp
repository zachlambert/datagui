#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/tree/systems.hpp"
#include "datagui/tree/tree.hpp"
#include "datagui/visual/window.hpp"

namespace datagui {

class Gui {
public:
  Gui(const Window::Config& window_config = Window::Config());

  bool running() const;
  void render_begin();
  void render_end();

  bool linear_layout();
  void container_end();

  void text(const std::string& text, const std::function<void(TextStyle&)>& set_style);

#if 0
  DataPtr<std::string> text_input(const std::string& initial_text);
#endif

private:
  Window window;
  FontManager font_manager;
  Tree tree;
  Systems systems;
};

} // namespace datagui
