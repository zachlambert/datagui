#include "datagui/input/event.hpp"
#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  std::vector<std::string> choices;
  for (std::size_t i = 0; i < 20; i++) {
    choices.push_back("Item " + std::to_string(i));
  }

  while (gui.running()) {
    if (gui.series()) {
      auto window_1_open = gui.variable<bool>(false);
      gui.button("Open window 1", [=]() { window_1_open.set(true); });

      gui.args().bg_color(datagui::Color::Hsl(180, 0.7, 0.7));
      if (gui.floating(window_1_open, "Floating 1", 400, 100)) {
        if (gui.series()) {
          gui.text_input("Input", {});
          gui.text_box("Here is some text");
          gui.text_box("Foo");
          gui.text_box("Bar");
          gui.text_box("Baz");
          gui.text_box("etc...");
          gui.text_box("last line???");
          gui.end();
        }
        gui.end();
      }

      auto window_2_open = gui.variable<bool>(false);
      gui.button("Open window 2", [=]() { window_2_open.set(true); });

      gui.args().bg_color(datagui::Color::Hsl(0, 0.7, 0.7));
      if (gui.floating(window_2_open, "Floating 2", 300, 150)) {
        gui.end();
      }

      gui.text_box("Select color");
      gui.dropdown(choices, -1, [&](int choice) {
        std::cout << "Selected '" << choices[choice] << "'" << std::endl;
      });
      gui.dropdown({}, -1, {});
      gui.text_box("Text below");
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
