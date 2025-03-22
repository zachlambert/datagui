#include <datagui/gui.hpp>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    gui.render_begin();
    if (gui.linear_layout()) {
      gui.text("Hello", [](auto& style) {
        style.text_color = datagui::Color::Red();
      });
      gui.text("Foo", [](auto& style) { style.font_size = 30; });
      gui.text("Bar");
      gui.container_end();
    }
    gui.render_end();
  }
  return 0;
}
