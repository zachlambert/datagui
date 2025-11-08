#include "datagui/input/event.hpp"
#include <datagui/gui.hpp>
#include <iostream>

int main() {
  DATAGUI_LOG_INIT();

  datagui::Gui gui;

  std::vector<std::string> choices;
  for (std::size_t i = 0; i < 20; i++) {
    choices.push_back("Item " + std::to_string(i));
  }

  while (gui.running()) {
    if (gui.begin()) {
      if (gui.series_begin()) {
        auto window_1_open = gui.variable<bool>(false);
        if (gui.button("Open window 1") && !*window_1_open) {
          window_1_open.set(!*window_1_open);
        }
        gui.args_floating().bg_color(datagui::Color::Hsl(180, 0.7, 0.7));
        if (gui.floating_begin(window_1_open, "Floating 1", 400, 100)) {
          if (gui.series_begin()) {
            gui.text_input("Input");
            gui.text_box("Here is some text");
            gui.text_box("Foo");
            gui.text_box("Bar");
            gui.text_box("Baz");
            gui.text_box("etc...");
            gui.text_box("last line???");
            gui.series_end();
          }
          gui.floating_end();
        }

        auto window_2_open = gui.variable<bool>(false);
        if (gui.button("Open window 2") && !*window_2_open) {
          window_2_open.set(true);
        }
        gui.args_floating().bg_color(datagui::Color::Hsl(0, 0.7, 0.7));
        if (gui.floating_begin(window_2_open, "Floating 2", 300, 150)) {
          gui.floating_end();
        }

        gui.text_box("Select color");
        if (auto choice = gui.dropdown(choices, -1)) {
          std::cout << "Selected '" << choices[*choice] << "'" << std::endl;
        }
        gui.dropdown({}, -1);
        gui.text_box("Text below");
        gui.series_end();
      }
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
