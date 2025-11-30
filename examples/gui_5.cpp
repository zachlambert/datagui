#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  while (gui.running()) {
    if (gui.series()) {
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
      gui.end();
    }
    gui.poll();
  }
  return 0;
}
