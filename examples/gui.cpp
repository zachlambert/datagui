#include <datagui/gui.hpp>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    gui.begin();
    if (gui.linear_layout()) {
      gui.text("Welcome Screen!", [](auto& style) {
        style.text_color = datagui::Color::Red();
        style.font_size = 40;
      });

      auto name = gui.data<std::string>();
      if (gui.linear_layout([](datagui::LinearLayoutStyle& style) {
            style.direction = datagui::LinearLayoutDirection::Horizontal;
            style.length = -1;
            style.width = 0;
          })) {
        gui.text("Name: ", [](auto& style) { style.font_size = 30; });
        auto data = gui.text_input([](auto& style) { style.max_width = -1; });
        if (data.modified()) {
          name.mut() = *data;
        }
        gui.container_end();
      }

      if (name->empty()) {
        gui.text("Hello... what is your name?", [](auto& style) {
          style.font_size = 30;
        });
      } else {
        gui.text("Hello " + *name, [](auto& style) { style.font_size = 30; });
      }
      gui.container_end();
    }
    gui.end();
  }
  return 0;
}
