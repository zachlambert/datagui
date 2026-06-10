#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  while (gui.poll()) {
    gui.group();
    DATAGUI_SCOPE(gui);

    gui.text_box("Hello");
    if (gui.button("Click Me")) {
      std::cout << "Clicked" << std::endl;
    }
  }
  return 0;
}
