#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  std::vector<std::string> choices;
  for (std::size_t i = 0; i < 20; i++) {
    choices.push_back("-------- Item " + std::to_string(i));
  }

  while (gui.poll()) {
    gui.group();
    DATAGUI_SCOPE(gui);

    auto& window_1_open = gui.variable<bool>(false);
    if (gui.button("Open window 1 (retained)")) {
      window_1_open = true;
    }

    // Retained when closing
    gui.args().bg_color(datagui::Color::Hsl(180, 0.7, 0.7)).retain();
    if (gui.popup(window_1_open, "Floating 1", 700, 100)) {
      DATAGUI_SCOPE(gui);
      gui.text_input("Input");
      gui.text_box("Here is some text");
      gui.text_box("Foo");
      gui.text_box("Bar");
      gui.text_box("Baz");
      gui.text_box("etc...");
      gui.text_box("last line???");
    }

    auto& window_2_open = gui.variable<bool>(false);
    if (gui.button("Open window 2 (not retained)")) {
      window_2_open = true;
    }

    // Not retained when closing
    gui.args().bg_color(datagui::Color::Hsl(0, 0.7, 0.7));
    if (gui.popup(window_2_open, "Floating 2", 300, 150)) {
      DATAGUI_SCOPE(gui);
      if (auto choice = gui.select(-1, choices)) {
        std::cout << "Popup 2 Selected '" << choices[*choice] << "'"
                  << std::endl;
      }
    }

    gui.text_box("Select color");
    if (auto choice = gui.select(-1, choices)) {
      std::cout << "Selected '" << choices[*choice] << "'" << std::endl;
    };
    gui.select(-1, {});
    gui.text_box("Text below");
  }
  return 0;
}
