#include <datagui/gui.hpp>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout()) {
      gui.text_box("Test 1");
      gui.text_input();
      gui.text_box("Test 2");
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}
