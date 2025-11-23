#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.series()) {
      std::cout << "Revisit" << std::endl;
      gui.text_box("Hello");
      gui.button("Click Me", []() { //
        std::cout << "Clicked" << std::endl;
      });
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
