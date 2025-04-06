#include <datagui/gui.hpp>

int main() {
  datagui::Gui gui;

  auto style_h1 = [](datagui::TextStyle& style) {
    style.text_color = datagui::Color::Red();
    style.font_size = 40;
  };

  auto style_text = [](datagui::TextStyle& style) { style.font_size = 30; };

  auto style_horiz_expand = [](datagui::LinearLayoutStyle& style) {
    style.direction = datagui::LinearLayoutDirection::Horizontal;
    style.length = -1;
    style.width = 0;
  };

  auto style_text_input_expand = [](datagui::TextInputStyle& style) {
    style.max_width = -1;
  };

  while (gui.running()) {
    gui.begin();
    if (gui.linear_layout()) {
      gui.text("Welcome Screen!", style_h1);

      auto name = gui.data<std::string>();

      if (gui.linear_layout(style_horiz_expand)) {
        gui.text("Name: ", style_text);
        auto data = gui.text_input(style_text_input_expand);
        if (data.modified()) {
          name.mut() = *data;
        }
        gui.container_end();
      }

      if (name->empty()) {
        gui.text("Hello... what is your name?", style_text);
      } else {
        gui.text("Hello " + *name, style_text);
      }

      gui.container_end();
    }
    gui.end();
  }
  return 0;
}
