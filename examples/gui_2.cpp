#include <chrono>
#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  auto timer = gui.variable<int>(0);
  using clock_t = std::chrono::high_resolution_clock;
  auto next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));

  auto timer_paused = gui.variable<bool>(false);

  while (gui.running()) {
    if (gui.begin()) {
      if (gui.series_begin()) {
        gui.text_box("Welcome Screen!");

        auto name = gui.variable<std::string>("");

        if (gui.series_begin()) {
          gui.text_box("Name: ");
          gui.depend_variable(name);
          gui.text_input(name);
          gui.series_end();
        }
        gui.checkbox(false, [](bool value) {
          std::cout << "Checked: " << value << std::endl;
        });

        if (name->empty()) {
          gui.text_box("Hello... what is your name?");
        } else {
          gui.text_box("Hello " + *name);
        }

        gui.args_series().length_fixed(100);
        if (gui.series_begin()) {
          for (std::size_t i = 0; i < 10; i++) {
            gui.text_input("Item " + std::to_string(i));
          }
          gui.series_end();
        }

        if (gui.series_begin()) {
          const std::vector<std::string> colors = {"red", "green", "blue"};
          gui.dropdown(colors, 0, [colors](int choice) {
            std::cout << "Selected " << colors[choice] << "!" << std::endl;
          });
          gui.series_end();
        }

        gui.depend_variable(timer);
        gui.text_box("Timer: " + std::to_string(*timer));

        gui.button("Reset", [&]() {
          timer.set(0);
          next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));
        });

        if (gui.series_begin()) {
          gui.checkbox(timer_paused);
          gui.text_box("Paused");
          gui.series_end();
        }

        if (gui.section_begin("Open me")) {
          if (gui.series_begin()) {
            gui.text_box("Hello :)");
            if (gui.labelled_begin("Input")) {
              gui.text_input("");
              gui.labelled_end();
            }
            gui.series_end();
          }
          gui.section_end();
        }

        gui.series_end();
      }
      gui.end();
    }
    gui.poll();

    if (clock_t::now() > next_t) {
      if (!*timer_paused) {
        timer.set(*timer + 1);
      }
      next_t += clock_t::duration(std::chrono::seconds(1));
    }
  }
  return 0;
}
