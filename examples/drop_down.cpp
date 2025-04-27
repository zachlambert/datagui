#include <datagui/gui.hpp>
#include <iostream>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  auto style_root = [](datagui::VerticalLayoutStyle& style) {
    style.padding = 10;
    style.inner_padding = 10;
  };

  auto style_drop_down = [](datagui::DropDownStyle& style) {
    style.padding = 20;
    style.border_width = 10;
    style.inner_border_width = 5;
    style.bg_color = datagui::Color::Gray(0.7);
    style.border_color = datagui::Color::Black();
  };

  std::vector<std::string> choices = {"red", "green", "blue"};

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(style_root)) {
      gui.text_box("Select color");
      if (auto choice = gui.drop_down(choices, -1, style_drop_down)) {
        std::cout << "Selected '" << choices[*choice] << "'" << std::endl;
      }
      gui.text_box("Text below");
      gui.layout_end();
    }
    gui.end();
  }
  return 0;
}
