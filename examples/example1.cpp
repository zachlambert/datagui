#include <datagui/window.hpp>
#include <iostream>

int main() {
  datagui::Window::Config config;
  config.title = "Datagui Example 1";
  datagui::Style style;

  datagui::Window window(config, style);
  bool checkbox1 = false;
  std::string text3;

  while (window.running()) {
    window.render_begin();
    if (window.vertical_layout()) {
      window.text("Line 1");
      window.text("Line 2");
      if (window.button("Click me!")) {
        std::cout << "Clicked!" << std::endl;
      }
      if (window.horizontal_layout()) {
        if (auto value = window.checkbox()) {
          checkbox1 = *value;
          std::cout << "Toggle -> " << (*value ? "on" : "off") << std::endl;
        }
        window.text("Toggle me!");
        window.container_end();
      }
      if (window.optional(checkbox1)) {
        printf("Hello\n");
        window.text("Hello");
        window.container_end();
      }
      if (auto value = window.text_input("min input", 0)) {
        std::cout << "Input 1 changed -> " << *value << std::endl;
      }
      if (auto value = window.text_input("expand input", -1)) {
        std::cout << "Input 2 changed -> " << *value << std::endl;
      }
      if (auto value = window.text_input("finite input", 200)) {
        text3 = *value;
        std::cout << "Input 3 changed -> " << *value << std::endl;
      }
      if (!text3.empty()) {
        if (window.button(text3, 0)) {
          std::cout << "Clicked another button" << std::endl;
        }
      } else {
        window.checkbox();
      }
      static const std::vector<std::string> choices = {"red", "green", "blue"};
      if (auto value = window.selection(choices)) {
        if (*value == -1) {
          std::cout << "Selection -> <none>" << std::endl;
        } else {
          std::cout << "Selection -> " << choices[*value] << std::endl;
        }
      }
      window.text("some more");
      window.text("text goes");
      window.text("here okay");

      window.container_end();
    }
    window.render_end();
  }
  return 0;
}
