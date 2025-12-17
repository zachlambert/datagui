#include "datagui/visual/window.hpp"

#include <GLFW/glfw3.h>
#include <assert.h>
#include <cstring>
#include <mutex>
#include <shared_mutex>

namespace datagui {

static std::vector<std::pair<GLFWwindow*, Window*>> windows;
static std::shared_mutex windows_mutex;

static Window* lookup_window(GLFWwindow* glfw_window) {
  std::shared_lock<std::shared_mutex> lock;
  for (auto [glfw_window_i, window_i] : windows) {
    if (glfw_window == glfw_window_i) {
      return window_i;
    }
  }
  return nullptr;
}

void glfw_mouse_button_callback(
    GLFWwindow* glfw_window,
    int button,
    int action,
    int mods) {
  auto window = lookup_window(glfw_window);
  if (!window) {
    return;
  }

  MouseEvent event;

  switch (button) {
  case GLFW_MOUSE_BUTTON_LEFT:
    event.button = MouseButton::Left;
    break;
  case GLFW_MOUSE_BUTTON_MIDDLE:
    event.button = MouseButton::Middle;
    break;
  case GLFW_MOUSE_BUTTON_RIGHT:
    event.button = MouseButton::Right;
    break;
  default:
    return;
  }

  switch (action) {
  case GLFW_PRESS:
    event.action = MouseAction::Press;
    window->mouse_button_down_[(std::size_t)event.button] = true;
    break;
  case GLFW_RELEASE:
    event.action = MouseAction::Release;
    window->mouse_button_down_[(std::size_t)event.button] = false;
    break;
  default:
    return;
  }

  double mx, my;
  glfwGetCursorPos(glfw_window, &mx, &my);
  event.position = Vec2(mx, my);

  window->mouse_events_.push_back(event);
}

void glfw_scroll_callback(
    GLFWwindow* glfw_window,
    double x_offset,
    double y_offset) {
  (void)x_offset;

  auto window = lookup_window(glfw_window);
  if (!window) {
    return;
  }

  ScrollEvent event;
  event.amount = y_offset * -50;
  // Set position within Gui
  window->scroll_events_.push_back(event);
}

void glfw_key_callback(
    GLFWwindow* glfw_window,
    int key,
    int scancode,
    int action,
    int mods) {
  auto window = lookup_window(glfw_window);
  if (!window) {
    return;
  }

  KeyEvent event;

  switch (action) {
  case GLFW_PRESS:
    event.action = KeyAction::Press;
    break;
  case GLFW_REPEAT:
    event.action = KeyAction::Repeat;
    break;
  case GLFW_RELEASE:
    event.action = KeyAction::Release;
    break;
  default:
    return;
  }

  switch (key) {
  case GLFW_KEY_LEFT:
    event.key = Key::Left;
    break;
  case GLFW_KEY_RIGHT:
    event.key = Key::Right;
    break;
  case GLFW_KEY_UP:
    event.key = Key::Up;
    break;
  case GLFW_KEY_DOWN:
    event.key = Key::Down;
    break;
  case GLFW_KEY_TAB:
    event.key = Key::Tab;
    break;
  case GLFW_KEY_ESCAPE:
    event.key = Key::Escape;
    break;
  case GLFW_KEY_ENTER:
    event.key = Key::Enter;
    break;
  case GLFW_KEY_BACKSPACE:
    event.key = Key::Backspace;
    break;
  case GLFW_KEY_DELETE:
    event.key = Key::Delete;
    break;
  default:
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
      event.key = (Key)((int)Key::A + (key - GLFW_KEY_A));
    } else {
      return;
    }
    break;
  }

  event.mod.ctrl = mods & (1 << 1);
  event.mod.shift = mods & (1 << 0);

  event.glfw_window = (void*)glfw_window;

  window->key_events_.push_back(event);
}

void glfw_char_callback(GLFWwindow* glfw_window, unsigned int codepoint) {
  auto window = lookup_window(glfw_window);
  if (!window) {
    return;
  }

  if (codepoint >= 256) {
    return;
  }

  TextEvent event;
  event.value = char(codepoint);
  window->text_events_.push_back(event);
}

Window::Window(const Config& config) :
    config(config), window(nullptr), size_(config.width, config.height) {
  for (std::size_t i = 0; i < MouseButtonSize; i++) {
    mouse_button_down_[i] = false;
  }
  open();
  std::scoped_lock<std::shared_mutex> lock(windows_mutex);
  windows.emplace_back(window, this);
}

Window::~Window() {
  if (window) {
    {
      std::scoped_lock<std::shared_mutex> lock(windows_mutex);
      for (auto iter = windows.begin(); iter != windows.end(); iter++) {
        if (iter->second == this) {
          assert(iter->first == window);
          windows.erase(iter);
          break;
        }
      }
    }
    close();
  }
}

bool Window::running() const {
  return window && !glfwWindowShouldClose(window);
}

void Window::open() {
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize glfw");
  }

  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Create window with graphics context
  window = glfwCreateWindow(
      config.width,
      config.height,
      config.title.c_str(),
      nullptr,
      nullptr);

  if (window == nullptr) {
    throw std::runtime_error("Failed to create window");
  }

  glfwMakeContextCurrent(window);

  if (config.vsync) {
    glfwSwapInterval(1);
  } else {
    glfwSwapInterval(0);
  }

  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("Failed to initialise glew");
  }

  glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
  glfwSetScrollCallback(window, glfw_scroll_callback);
  glfwSetKeyCallback(window, glfw_key_callback);
  glfwSetCharCallback(window, glfw_char_callback);
}

void Window::close() {
  if (!window) {
    return;
  }

  glfwMakeContextCurrent(window);
  glfwDestroyWindow(window);
  glfwTerminate();
  window = nullptr;
}

void Window::render_begin() {
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  size_ = Vec2(display_w, display_h);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(1.f, 1.f, 1.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::render_end() {
  glfwSwapBuffers(window);
}

void Window::poll_events() {
  mouse_events_.clear();
  scroll_events_.clear();
  key_events_.clear();
  text_events_.clear();
  glfwPollEvents();

  Modifiers mod;
  mod.ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
  mod.shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

  double mx, my;
  glfwGetCursorPos(window, &mx, &my);
  mouse_pos_ = Vec2(mx, my);

  for (auto& event : mouse_events_) {
    event.mod = mod;
  }
  for (auto& event : scroll_events_) {
    event.position = mouse_pos_;
    event.mod = mod;
  }

  MouseEvent hold_event;
  hold_event.action = MouseAction::Hold;
  hold_event.position = mouse_pos_;
  hold_event.mod = mod;

  for (std::size_t i = 0; i < MouseButtonSize; i++) {
    if (!mouse_button_down_[i]) {
      continue;
    }
    hold_event.button = (MouseButton)i;
    mouse_events_.push_back(hold_event);
  }

  for (auto& event : mouse_events_) {
    if (event.action != MouseAction::Press) {
      continue;
    }
    auto& prev = last_mouse_press_time_[(std::size_t)event.button];
    auto now = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev)
                    .count() *
                1e-9;
    prev = now;

    if (dt < double_click_time) {
      event.is_double_click = true;
      // Reset prev to avoid a second double click for a third rapid click
      prev = std::chrono::high_resolution_clock::time_point();
    } else {
      prev = now;
    }
  }
}

} // namespace datagui
