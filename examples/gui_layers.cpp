#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  datagui::Style style_root;
  style_root.series_vertical();
  style_root.series_width_dynamic();
  style_root.series_outer_padding(10);
  style_root.series_inner_padding(10);

  datagui::Style style_dropdown;
  style_dropdown.text_padding(5);
  style_dropdown.dropdown_border_width(2);
  style_dropdown.dropdown_inner_border_width(2);
  style_dropdown.dropdown_bg_color(datagui::Color::Gray(0.7));
  style_dropdown.dropdown_border_color(datagui::Color::Black());

  datagui::Style style_input = datagui::Style().text_input_width_dynamic();

  datagui::Style style_floating_1;
  style_floating_1.floating_absolute({150, 50, 150, 150});
  style_floating_1.floating_bg_color(datagui::Color::Hsl(270, 0.9, 0.6));
  style_floating_1.title_bar_enable(true);
  style_floating_1.close_button_enable(true);

  datagui::Style style_floating_2;
  style_floating_2.floating_absolute({200, 200, 200, 50});
  style_floating_2.floating_bg_color(datagui::Color::Hsl(900, 0.9, 0.6));
  style_floating_2.title_bar_enable(true);
  style_floating_2.close_button_enable(true);

  std::vector<std::string> choices = {"red", "green", "blue"};

  while (gui.running()) {
    gui.begin();
    if (gui.series_begin(style_root)) {
      auto window_1_open = gui.variable<bool>(false);
      if (gui.button("Open window 1") && !*window_1_open) {
        window_1_open.set(true);
      }
      if (gui.floating_begin(window_1_open, "Floating 1", style_floating_1)) {
        gui.text_input("Input", style_input);
        gui.text_box("Here is some text");
        gui.floating_end();
      }

      auto window_2_open = gui.variable<bool>(false);
      if (gui.button("Open window 2") && !*window_2_open) {
        window_2_open.set(true);
      }
      if (gui.floating_begin(window_2_open, "Floating 2", style_floating_2)) {
        gui.floating_end();
      }

      gui.text_box("Select color");
      if (auto choice = gui.dropdown(choices, -1, style_dropdown)) {
        std::cout << "Selected '" << choices[*choice] << "'" << std::endl;
      }
      gui.dropdown({}, -1, style_dropdown);
      gui.text_box("Text below");
      gui.series_end();
    }
    gui.end();
  }
  return 0;
}
