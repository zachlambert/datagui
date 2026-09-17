#include <datagui/gui.hpp>
#include <algorithm>
#include <iostream>

class NumberDisplay {
public:
  void visit(dgui::Gui& gui, int number) {
    gui.text_box("The number is " + std::to_string(number));
    for (int i = 0; i < std::clamp(number, 0, 10); i++) {
      gui.key(i);
      if (gui.button("Button " + std::to_string(i))) {
        std::cout << "Pressed " << i << std::endl;
      }
    }
    // Set a key to separate from the above button
    gui.key(11);
    std::ignore = gui.text_input("Type here...");

    if (number <= 0) {
      gui.key(12);
      gui.text_box("No buttons shown");
    } else if (number >= 10) {
      gui.key(12);
      gui.text_box("Max buttons shown");
    }
  }
};

int main() {
  dgui::Gui gui;
  gui.open();

  int number = 2;
  while (gui.poll()) {
    gui.group();
    DGUI_SCOPE(gui);

    gui.number_input_v(number);
    gui.component<NumberDisplay>(number);
  }
  return 0;
}
