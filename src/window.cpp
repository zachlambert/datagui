#include "datagui/window.hpp"

#include "datagui/exception.hpp"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <cstring>

namespace datagui {

using namespace std::placeholders;

struct Events {
  struct {
    int action = -1;
    int button = 0;
  } mouse;
  struct {
    int action = -1;
    int key = 0;
    int mods = 0;
  } key;
  struct {
    bool received = false;
    char character = 0;
  } text;

  void reset() {
    mouse.action = -1;
    key.action = -1;
    text.received = false;
  }
} events;

void glfw_mouse_button_callback(GLFWwindow* callback_window, int button, int action, int mods) {
  events.mouse.action = action;
  events.mouse.button = button;
}

void glfw_key_callback(GLFWwindow* callback_window, int key, int scancode, int action, int mods) {
  events.key.action = action;
  events.key.key = key;
  events.key.mods = mods;
}

void glfw_char_callback(GLFWwindow* callback_window, unsigned int codepoint) {
  if (codepoint < 256) {
    events.text.received = true;
    events.text.character = char(codepoint);
  }
}

Window::Window(const Config& config, const Style& style) :
    config(config),
    style(style),
    window(nullptr),
    window_size(Vecf::Zero()),
    buttons(this->style, font),
    checkboxes(this->style, font),
    linear_layouts(this->style),
    selections(this->style, font),
    options(this->style, font),
    texts(this->style, font),
    text_inputs(this->style, font) {
  tree.register_element(Element::Button, buttons);
  tree.register_element(Element::Checkbox, checkboxes);
  tree.register_element(Element::LinearLayout, linear_layouts);
  tree.register_element(Element::Selection, selections);
  tree.register_element(Element::Option, options);
  tree.register_element(Element::Text, texts);
  tree.register_element(Element::TextInput, text_inputs);
  open();
}

Window::~Window() {
  if (window) {
    close();
  }
}

bool Window::running() const { return window && !glfwWindowShouldClose(window); }

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

  font = load_font(style.text.font, style.text.font_size);
  renderers.geometry.init();
  renderers.text.init();

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

bool Window::vertical_layout(float length, float width, const std::string& key, bool open_always) {
  int node = tree.next(key, Element::LinearLayout, [&]() {
    return linear_layouts.create(length, width, LayoutDirection::Vertical);
  });
  if (tree[node].changed || open_always) {
    tree.down();
    return true;
  }
  return false;
}

bool Window::horizontal_layout(
    float length,
    float width,
    const std::string& key,
    bool open_always) {
  int node = tree.next(key, Element::LinearLayout, [&]() {
    return linear_layouts.create(length, width, LayoutDirection::Horizontal);
  });
  if (tree[node].changed || open_always) {
    tree.down();
    return true;
  }
  return false;
}

void Window::layout_end() { tree.up(); }

void Window::text(const std::string& text, float max_width, const std::string& key) {
  tree.next(key, Element::Text, [&]() { return texts.create(text, max_width); });
}

bool Window::button(const std::string& text, float max_width, const std::string& key) {
  int node = tree.next(key, Element::Button, [&]() { return buttons.create(text, max_width); });
  return tree[node].changed;
}

const bool* Window::checkbox(const std::string& key, bool ret_always) {
  int node = tree.next(key, Element::Checkbox, [&]() { return checkboxes.create(false); });
  if (tree[node].changed || ret_always) {
    return checkboxes.value(tree[node]);
  }
  return nullptr;
}

const std::string* Window::text_input(
    const std::string& default_text,
    float max_width,
    const std::string& key,
    bool ret_always) {
  int node = tree.next(key, Element::TextInput, [&]() {
    return text_inputs.create(max_width, default_text);
  });
  if (tree[node].changed || ret_always) {
    return text_inputs.value(tree[node]);
  }
  return nullptr;
}

const int* Window::selection(
    const std::vector<std::string>& choices,
    int default_choice,
    float max_width,
    const std::string& key,
    bool ret_always) {

  int node = tree.next(key, Element::Selection, [&]() {
    return selections.create(choices, default_choice, max_width);
  });

  bool changed = false;
  if (tree.node_in_focus_tree(node)) {
    tree.down();
    for (int i = 0; i < choices.size(); i++) {
      tree.next(choices[i], Element::Option, [&]() {
        return options.create(choices[i], max_width);
      });
      if (tree.current_node_changed()) {
        if (*selections.choice(tree[node]) != i) {
          selections.set_choice(tree[node], i);
          changed = true;
        }
      }
    }
    tree.up();
  } else {
    tree.down();
    tree.up();
  }

  if (changed || ret_always) {
    return selections.choice(tree[node]);
  }
  return nullptr;
}

void Window::hidden(const std::string& key) {
  //
  tree.next(key, Element::Undefined, nullptr);
}

void Window::render_begin() {
  glClearColor(style.bg_color.r, style.bg_color.g, style.bg_color.b, 1);
  glClearDepth(0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  window_size = Vecf(display_w, display_h);

  tree.begin();
}

void Window::render_end() {
  tree.end(window_size);

  event_handling();

  tree.render(renderers, window_size);

  if (tree.root_node() != -1) {
    const auto& root_node = tree[tree.root_node()];
    glfwSetWindowSizeLimits(window, root_node.fixed_size.x, root_node.fixed_size.y, -1, -1);
  }

  glfwSwapBuffers(window);
}

void Window::event_handling() {
  glfwPollEvents();

  double mx, my;
  glfwGetCursorPos(window, &mx, &my);
  Vecf mouse_pos(mx, my);

  if (events.mouse.action == GLFW_PRESS) {
    tree.mouse_press(mouse_pos);
  }
  if (events.mouse.action == GLFW_RELEASE) {
    tree.mouse_release(mouse_pos);
  }
  if (tree.node_held() != -1) {
    auto& node = tree[tree.node_held()];
    if (tree.get_elements(node).held(node, mouse_pos)) {
      tree.node_changed(node);
    }
  }
  if (events.key.action == GLFW_PRESS && events.key.key == GLFW_KEY_TAB) {
    tree.focus_next(events.key.mods & (1 << 0));
  }
  if (tree.node_focused() != -1) {
    auto& node = tree[tree.node_focused()];
    if (events.key.action == GLFW_PRESS || events.key.action == GLFW_REPEAT ||
        events.key.action == GLFW_RELEASE) {
      bool shift = events.key.mods & 1 << 0;
      bool ctrl = events.key.mods & 1 << 1;
      bool release = events.key.action == GLFW_RELEASE;
      bool changed = false;
      switch (events.key.key) {
      case GLFW_KEY_ESCAPE:
        tree.focus_leave(false);
        break;
      case GLFW_KEY_ENTER:
        changed = tree.get_elements(node).key_event(
            node,
            KeyEvent::key(KeyValue::Enter, release, shift, ctrl));
        break;
      case GLFW_KEY_LEFT:
        changed = tree.get_elements(node).key_event(
            node,
            KeyEvent::key(KeyValue::LeftArrow, release, shift, ctrl));
        break;
      case GLFW_KEY_RIGHT:
        changed = tree.get_elements(node).key_event(
            node,
            KeyEvent::key(KeyValue::RightArrow, release, shift, ctrl));
        break;
      case GLFW_KEY_BACKSPACE:
        changed = tree.get_elements(node).key_event(
            node,
            KeyEvent::key(KeyValue::Backspace, release, shift, ctrl));
        break;
      default:
        break;
      }
      if (changed) {
        tree.node_changed(node);
      }
    } else if (events.text.received) {
      bool changed = tree.get_elements(node).key_event(node, KeyEvent::text(events.text.character));
      if (changed) {
        tree.node_changed(node);
      }
    }
  }

  events.reset();
}

} // namespace datagui
