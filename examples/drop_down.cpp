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

  auto style_window = [](datagui::WindowStyle& style) {
    datagui::WindowPositionAbsolute position;
    position.margin = 100;
    style.position = position;
    style.bg_color = datagui::Color::Hsl(270, 0.9, 0.6);

    datagui::TitleBarStyle title_bar;
    style.title_bar = title_bar;
  };

  std::vector<std::string> choices = {"red", "green", "blue"};

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(style_root)) {
      if (gui.floating("Pop-up", style_window)) {
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
