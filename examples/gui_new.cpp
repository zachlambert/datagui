#include <datagui/gui.hpp>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.begin()) {
      if (gui.series_begin()) {
        gui.text_box("Hello");
        gui.series_end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
