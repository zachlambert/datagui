#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.begin()) {
      std::cout << "Revisit" << std::endl;
      if (gui.series_begin()) {
        gui.text_box("Hello");
        gui.button("Click Me", []() { //
          std::cout << "Clicked" << std::endl;
        });
        gui.series_end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
