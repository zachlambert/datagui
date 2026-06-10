#include <chrono>
#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  int timer = 0;
  bool timer_paused = false;

  using clock_t = std::chrono::high_resolution_clock;
  auto next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));

  while (gui.running()) {
    gui.group();
    {
      DATAGUI_SCOPE(gui);
      gui.text_box("Welcome Screen!");

      auto& name = gui.variable<std::string>("");

      gui.group();
      {
        DATAGUI_SCOPE(gui);
        gui.text_box("Name: ");
        gui.text_input_v(name);
      }
      if (auto value = gui.checkbox(false)) {
        std::cout << "Checked: " << *value << std::endl;
      };

      if (name.empty()) {
        gui.text_box("Hello... what is your name?");
      } else {
        gui.text_box("Hello " + name);
      }

      gui.args().height_fixed(100).border();
      gui.group();
      {
        DATAGUI_SCOPE(gui);
        for (std::size_t i = 0; i < 10; i++) {
          gui.text_input("Item " + std::to_string(i));
        }
      }

      const std::vector<std::string> colors = {"red", "green", "blue"};
      if (auto choice = gui.select(0, colors)) {
        std::cout << "Selected " << colors[*choice] << "!" << std::endl;
      };

      gui.text_box("Timer: " + std::to_string(timer));

      if (gui.button("Reset")) {
        timer = 0;
        next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));
      }

      gui.args().horizontal();
      gui.group();
      {
        DATAGUI_SCOPE(gui);
        gui.text_box("Paused");
        gui.checkbox_v(timer_paused);
      }

      if (gui.collapsable("Open me")) {
        DATAGUI_SCOPE(gui);
        gui.text_box("Hello :)");
        gui.args().horizontal();
        gui.group();
        {
          DATAGUI_SCOPE(gui);
          gui.text_box("Input");
          gui.text_input("");
        }
      }
    }
    gui.poll();

    if (clock_t::now() > next_t) {
      if (!timer_paused) {
        timer += 1;
      }
      next_t += clock_t::duration(std::chrono::seconds(1));
    }
  }
  return 0;
}
