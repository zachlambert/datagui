#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;
  using datagui::Color;

  while (gui.running()) {
    if (gui.group()) {
      gui.text_box("Number input");
      gui.number_input<int>(0, [](int value) {
        std::cout << "int: " << value << std::endl;
      });
      gui.number_input<float>(0, [](float value) {
        std::cout << "float: " << value << std::endl;
      });
      gui.number_input<double>(0, [](double value) {
        std::cout << "double: " << value << std::endl;
      });
      gui.number_input<std::size_t>(0, [](std::size_t value) {
        std::cout << "size_t: " << value << std::endl;
      });

      gui.text_box("Slider");
      gui.slider<int>(0, 0, 10, [](int value) {
        std::cout << "slider int: " << value << std::endl;
      });
      gui.slider<float>(0, 0, 1, [](float value) {
        std::cout << "slider float: " << value << std::endl;
      });
      gui.slider<double>(0, 0, 1, [](double value) {
        std::cout << "slider double: " << value << std::endl;
      });

      gui.text_box("Color picker");
      gui.color_picker(Color::Red(), [](const Color& value) {
        std::cout << "Color: " << value.r << ", " << value.g << ", " << value.b
                  << std::endl;
      });
      gui.text_box("Text below");

      gui.end();
    }
    gui.poll();
  }
  return 0;
}
