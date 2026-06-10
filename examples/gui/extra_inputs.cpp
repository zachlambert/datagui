#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;
  using datagui::Color;

  while (gui.poll()) {
    gui.group();
    DATAGUI_SCOPE(gui);

    gui.text_box("Number input");
    if (auto value = gui.number_input<int>(0)) {
      std::cout << "int: " << *value << std::endl;
    }
    if (auto value = gui.number_input<float>(0)) {
      std::cout << "float: " << *value << std::endl;
    }
    if (auto value = gui.number_input<double>(0)) {
      std::cout << "double: " << *value << std::endl;
    }
    if (auto value = gui.number_input<size_t>(0)) {
      std::cout << "size_t: " << *value << std::endl;
    }

    gui.text_box("Slider");
    if (auto value = gui.slider<int>(0, 0, 10)) {
      std::cout << "slider int: " << *value << std::endl;
    };
    if (auto value = gui.slider<float>(0, 0, 1)) {
      std::cout << "slider float: " << *value << std::endl;
    };
    gui.args().always();
    if (auto value = gui.slider<double>(0, 0, 1)) {
      std::cout << "slider double: " << *value << std::endl;
    };

    gui.text_box("Color picker");
    if (auto value = gui.color_picker(Color::Red())) {
      std::cout << "Color: " << value->r << ", " << value->g << ", " << value->b
                << std::endl;
    };
    gui.text_box("Text below");
  }
  return 0;
}
