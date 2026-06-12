#include <datagui/gui.hpp>
#include <iostream>

int main() {
  dgui::Gui gui;
  gui.open();

  while (gui.poll()) {
    gui.group();
    DGUI_SCOPE(gui);

    gui.text_box("Hello");
    if (gui.button("Click Me")) {
      std::cout << "Clicked" << std::endl;
    }
  }
  return 0;
}
