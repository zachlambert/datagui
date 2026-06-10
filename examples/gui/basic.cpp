#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    gui.group();
    {
      DATAGUI_SCOPE(gui);
      gui.text_box("Hello");
      if (gui.button("Click Me")) {
        std::cout << "Clicked" << std::endl;
      }
    }

    gui.poll();
  }
  return 0;
}
