#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "datagui/geometry.hpp"
#include "datagui/internal/renderers.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/style.hpp"

#include "datagui/element/button.hpp"
#include "datagui/element/checkbox.hpp"
#include "datagui/element/linear_layout.hpp"
#include "datagui/element/selection.hpp"
#include "datagui/element/text.hpp"
#include "datagui/element/text_input.hpp"

namespace datagui {

class Window {
public:
  struct Config {
    std::string title;
    int width;
    int height;
    bool vsync;
    bool resizable;
    Config()
        : title("datagui"), width(900), height(600), vsync(false),
          resizable(true) {}
  };

  Window(const Config &config = Config(), const Style &style = Style());
  ~Window();

  bool running() const;
  void render_begin();
  void render_end();

  bool vertical_layout(float width = 0, float height = 0,
                       const std::string &key = "");

  bool horizontal_layout(float width = 0, float height = 0,
                         const std::string &key = "");

  void layout_end();

  void text(const std::string &text, float max_width = 0,
            const std::string &key = "");

  bool button(const std::string &text, float max_width = 0,
              const std::string &key = "");

  const bool *checkbox(const std::string &key = "");

  const std::string *text_input(const std::string &default_text = "",
                                float max_width = -1,
                                const std::string &key = "");

  const int *selection(const std::vector<std::string> &choices,
                       int default_choice = 0, float max_width = -1,
                       const std::string &key = "");

  void hidden(const std::string &key = "");

private:
  void open();
  void close();
  void event_handling();

  const Config config;
  const Style style;
  GLFWwindow *window;
  Vecf window_size;

  FontStructure font;
  Renderers renderers;

  Tree tree;
  ButtonSystem buttons;
  CheckboxSystem checkboxes;
  LinearLayoutSystem linear_layouts;
  SelectionSystem selections;
  TextSystem texts;
  TextInputSystem text_inputs;
};

} // namespace datagui
