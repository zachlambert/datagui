#include <datagui/gui.hpp>
#include <iostream>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  datagui::Style style_root;
  style_root.padding(10);
  style_root.layout_inner_padding(10);
  style_root.width_dynamic();

  datagui::Style style_dropdown;
  style_dropdown.padding(5);
  style_dropdown.border_width(2);
  style_dropdown.dropdown_inner_border_width(2);
  style_dropdown.bg_color(datagui::Color::Gray(0.7));
  style_dropdown.border_color(datagui::Color::Black());

  datagui::Style style_input;
  style_input.width_dynamic();

  datagui::Style style_floating_1;
  style_floating_1.float_absolute({150, 50, 150, 150});
  style_floating_1.bg_color(datagui::Color::Hsl(270, 0.9, 0.6));
  style_floating_1.close_button_enable(true);

  datagui::Style style_floating_2;
  style_floating_2.float_absolute({200, 200, 200, 50});
  style_floating_2.bg_color(datagui::Color::Hsl(900, 0.9, 0.6));
  style_floating_2.close_button_enable(true);

  std::vector<std::string> choices = {"red", "green", "blue"};

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(style_root)) {
      auto window_1_open = gui.variable<bool>(false);
      if (gui.button("Open window 1") && !*window_1_open) {
        window_1_open.set(true);
      }
      if (gui.floating(window_1_open, "Floating 1", style_floating_1)) {
        gui.text_input("Input", style_input);
        gui.text_box("Here is some text");
        gui.layout_end();
      }

      auto window_2_open = gui.variable<bool>(false);
      if (gui.button("Open window 2") && !*window_2_open) {
        window_2_open.set(true);
      }
      if (gui.floating(window_2_open, "Floating 2", style_floating_2)) {
        gui.layout_end();
      }

      gui.text_box("Select color");
      if (auto choice = gui.dropdown(choices, -1, style_dropdown)) {
        std::cout << "Selected '" << choices[*choice] << "'" << std::endl;
      }
      gui.dropdown({}, -1, style_dropdown);
      gui.text_box("Text below");
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}
