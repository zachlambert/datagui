#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.begin()) {
      std::cout << "Revisit" << std::endl;
      if (gui.series()) {
        gui.text_box("Hello");
        gui.button("Click Me", []() { //
          std::cout << "Clicked" << std::endl;
        });
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
