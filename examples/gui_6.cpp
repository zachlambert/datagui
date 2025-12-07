#include <datagui/gui.hpp>

int main() {
  datagui::Gui gui;
  using datagui::Color;

  while (gui.running()) {
    if (gui.vsplit(0.3)) {
      if (gui.group()) {
        auto counter = gui.variable<int>(0);
        if (gui.tabs({"Display", "Increment", "Decrement"})) {
          gui.args().width_expand().height_expand();
          if (gui.group()) {
            gui.text_box(std::to_string(*counter));
            gui.end();
          }
          gui.button("Increment", [counter]() { counter.mut()++; });
          gui.button("Decrement", [counter]() { counter.mut()--; });
          gui.end();
        }
        gui.text_box("Second");
        gui.end();
      }
      gui.args().width_expand();
      if (gui.hsplit(0.5)) {
        gui.text_box("Third");
        gui.args().split_fixed();
        if (gui.vsplit(0.6)) {
          gui.args().width_expand();
          if (gui.group()) {
            gui.text_box("Fourth");
            for (std::size_t i = 0; i < 20; i++) {
              gui.text_box("Fourth [" + std::to_string(i) + "]");
            }
            gui.end();
          }
          gui.text_box("Fifth");
          gui.end();
        }
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
