#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;
  using datagui::Color;

  while (gui.running()) {
    if (gui.vsplit(0.3)) {
      if (gui.group()) {
        gui.text_box("First");
        gui.text_box("Second");
        gui.end();
      }
      if (gui.group()) {
        gui.text_box("Third");
        gui.text_box("Fourth");
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
