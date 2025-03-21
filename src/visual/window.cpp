#include "datagui/visual/window.hpp"

#include "datagui/exception.hpp"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <cstring>

namespace datagui {

static Events g_events;

void glfw_mouse_button_callback(GLFWwindow* callback_window, int button, int action, int mods) {
  switch (action) {
  case GLFW_PRESS:
    g_events.mouse.press = true;
    break;
  case GLFW_RELEASE:
    g_events.mouse.release = true;
    break;
  }
}

void glfw_key_callback(GLFWwindow* callback_window, int key, int scancode, int action, int mods) {
  switch (action) {
  case GLFW_PRESS:
    g_events.mouse.press = true;
    break;
  case GLFW_RELEASE:
    g_events.mouse.release = true;
    break;
  }
  g_events.key.key = (Key)key;
  g_events.key.mod_ctrl = mods & (1 << 1);
  g_events.key.mod_shift = mods & (1 << 0);
}

void glfw_char_callback(GLFWwindow* callback_window, unsigned int codepoint) {
  if (codepoint < 256) {
    g_events.text.received = true;
    g_events.text.character = char(codepoint);
  }
}

Window::Window(const Config& config) :
    config(config), window(nullptr), window_size(Vecf::Zero()), events_(g_events) {
  open();
}

Window::~Window() {
  if (window) {
    close();
  }
}

bool Window::running() const {
  return window && !glfwWindowShouldClose(window);
}

void Window::open() {
  if (!glfwInit()) {
    throw InitializationError("Failed to initialize glfw");
  }

  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create window with graphics context
  window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);

  if (window == nullptr) {
    throw InitializationError("Failed to create window");
  }

  glfwMakeContextCurrent(window);

  if (config.vsync) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }

  if (glewInit() != GLEW_OK) {
    throw InitializationError("Failed to initialise glew");
  }

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);

  glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
  glfwSetKeyCallback(window, glfw_key_callback);
  glfwSetCharCallback(window, glfw_char_callback);
}

void Window::close() {
  if (!window) {
    throw WindowError("Window is already closed");
  }

  glfwMakeContextCurrent(window);
  glfwDestroyWindow(window);
  glfwTerminate();
  window = nullptr;
}

void Window::render_begin() {
  glClearColor(1.f, 1.f, 1.f, 1.f);
  glClearDepth(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  window_size = Vecf(display_w, display_h);
}

void Window::render_end() {
  glfwSwapBuffers(window);
}

void Window::poll_events() {
  events_.reset();
  glfwPollEvents();
}

} // namespace datagui
