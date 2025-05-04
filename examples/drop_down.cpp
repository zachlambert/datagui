#include <datagui/gui.hpp>
#include <iostream>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  auto style_root = [](datagui::VerticalLayoutStyle& style) {
    style.padding = 10;
    style.inner_padding = 10;
    style.width = 1.0_dynamic;
  };

  auto style_drop_down = [](datagui::DropDownStyle& style) {
    style.padding = 5;
    style.border_width = 2;
    style.inner_border_width = 2;
    style.bg_color = datagui::Color::Gray(0.7);
    style.border_color = datagui::Color::Black();
  };

  auto style_input = [](datagui::TextInputStyle& style) {
    style.width = datagui::LengthDynamic(1.0);
  };

  auto style_window_1 = [](datagui::WindowStyle& style) {
    datagui::WindowPositionAbsolute position;
    position.margin = 150;
    position.margin.left = 50;
    style.position = position;
    style.bg_color = datagui::Color::Hsl(270, 0.9, 0.6);

    datagui::TitleBarStyle title_bar;
    style.title_bar = title_bar;
  };

  auto style_window_2 = [](datagui::WindowStyle& style) {
    datagui::WindowPositionAbsolute position;
    position.margin = 200;
    position.margin.right = 50;
    style.position = position;
    style.bg_color = datagui::Color::Hsl(270, 0.9, 0.6);

    datagui::TitleBarStyle title_bar;
    style.title_bar = title_bar;
  };

  std::vector<std::string> choices = {"red", "green", "blue"};

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(style_root)) {
      auto window_1_open = gui.variable<bool>(false);
      if (gui.button("Open window 1") && !*window_1_open) {
        window_1_open.set(true);
      }
      if (gui.floating(window_1_open, "Window 1", style_window_1)) {
        gui.text_input("Input", style_input);
        gui.text_box("Here is some text");
        gui.layout_end();
      }

      auto window_2_open = gui.variable<bool>(false);
      if (gui.button("Open window 2") && !*window_2_open) {
        window_2_open.set(true);
      }
      if (gui.floating(window_2_open, "Window 2", style_window_2)) {
        gui.layout_end();
      }

      gui.text_box("Select color");
      if (auto choice = gui.drop_down(choices, -1, style_drop_down)) {
        std::cout << "Selected '" << choices[*choice] << "'" << std::endl;
      }
      gui.drop_down({}, -1, style_drop_down);
      gui.text_box("Text below");
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}
