#include <datagui/gui.hpp>

int main() {
  DATAGUI_LOG_INIT();
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.begin()) {
      if (gui.series_begin()) {
        gui.text_box("Hello");
        if (auto value = gui.text_input("")) {
          std::cout << "Hello " << *value << std::endl;
        }
        auto color = gui.variable<std::string>("red");
        gui.on_variable(color);
        gui.text_box("Color: " + *color);
        gui.text_input(color);
        gui.series_end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
