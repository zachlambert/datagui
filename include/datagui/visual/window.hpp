#pragma once

#include "datagui/geometry.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

namespace datagui {

enum class MouseButton { Left, Middle, Right };

enum class Key : unsigned int {
  Left = GLFW_KEY_LEFT,
  Right = GLFW_KEY_RIGHT,
  Up = GLFW_KEY_UP,
  Down = GLFW_KEY_DOWN,
  Tab = GLFW_KEY_TAB,
  Escape = GLFW_KEY_ESCAPE,
  Enter = GLFW_KEY_ENTER,
  Backspace = GLFW_KEY_BACKSPACE,
};

struct Events {
  struct {
    bool press = false;
    bool release = false;
    MouseButton button = MouseButton::Left;
  } mouse;

  struct {
    bool press = false;
    bool release = false;
    bool repeat = false;
    Key key;
    bool mod_ctrl = false;
    bool mod_shift = false;
  } key;

  struct {
    bool received = false;
    char character = 0;
  } text;

  void reset() {
    mouse.press = false;
    mouse.release = false;
    key.press = false;
    key.release = false;
    key.repeat = false;
    text.received = false;
  }
};

class Window {
public:
  struct Config {
    std::string title;
    int width;
    int height;
    bool vsync;
    bool resizable;
    Config() : title("datagui"), width(900), height(600), vsync(false), resizable(true) {}
  };

  Window(const Config& config = Config());
  ~Window();

  bool running() const;
  void render_begin();
  void render_end();

  const Vecf& size() const {
    return size_;
  }
  const Vecf& mouse_pos() const {
    return mouse_pos_;
  }

  void poll_events();
  const Events& events() const {
    return events_;
  }

private:
  void open();
  void close();

  const Config config;
  GLFWwindow* window;
  Vecf size_;

  Vecf mouse_pos_;
  Events& events_;
};

} // namespace datagui
