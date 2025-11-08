#include <datagui/gui.hpp>
#include <iostream>

int main() {
  DATAGUI_LOG_INIT();

  datagui::Gui gui;

  std::vector<std::string> choices = {"red", "green", "blue"};

  while (gui.running()) {
    if (gui.begin()) {
      if (gui.series_begin()) {
        auto window_1_open = gui.variable<bool>(false);
        if (gui.button("Open window 1")) {
          printf("Set open = %i\n", !*window_1_open);
          window_1_open.set(!*window_1_open);
        }
        if (gui.floating_begin(window_1_open, "Floating 1")) {
          if (gui.series_begin()) {
            gui.text_input("Input");
            gui.text_box("Here is some text");
            gui.series_end();
          }
          gui.floating_end();
        }

        auto window_2_open = gui.variable<bool>(false);
        if (gui.button("Open window 2") && !*window_2_open) {
          printf("Set open = %i\n", !*window_2_open);
          window_2_open.set(true);
        }
        if (gui.floating_begin(window_2_open, "Floating 2")) {
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
