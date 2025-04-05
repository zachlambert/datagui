#pragma once

#include "datagui/geometry.hpp"
#include "datagui/input/event.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <array>
#include <string>
#include <vector>

namespace datagui {

class Window {
public:
  struct Config {
    std::string title;
    int width;
    int height;
    bool vsync;
    bool resizable;
    Config() :
        title("datagui"),
        width(900),
        height(600),
        vsync(false),
        resizable(true) {}
  };

  Window(const Config& config = Config());
  ~Window();

  bool running() const;
  void render_begin();
  void render_end();

  const Vecf& size() const {
    return size_;
  }

  void poll_events();

  const std::vector<MouseEvent>& mouse_events() const {
    return mouse_events_;
  }
  const std::vector<KeyEvent>& key_events() const {
    return key_events_;
  }
  const std::vector<TextEvent>& text_events() const {
    return text_events_;
  }

private:
  void open();
  void close();

  const Config config;
  GLFWwindow* window;
  Vecf size_;

  std::vector<MouseEvent> mouse_events_;
  std::vector<KeyEvent> key_events_;
  std::vector<TextEvent> text_events_;
  std::array<bool, MouseButtonSize> mouse_button_down_;

  friend void glfw_mouse_button_callback(GLFWwindow*, int, int, int);
  friend void glfw_key_callback(GLFWwindow*, int, int, int, int);
  friend void glfw_char_callback(GLFWwindow*, unsigned int);
};

} // namespace datagui
