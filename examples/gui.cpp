#include <chrono>
#include <datagui/gui.hpp>
#include <iostream>

int main() {
  datagui::Gui gui;

  datagui::Style style_root;
  style_root.series_vertical();
  style_root.series_outer_padding({40, 10});
  style_root.series_inner_padding(10);

  datagui::Style style_h1;
  style_h1.text_color(datagui::Color::Red());
  style_h1.font_size(40);

  datagui::Style style_text;
  style_text.font_size(30);

  datagui::Style style_box;
  style_box.series_horizontal();
  style_box.series_width_dynamic();
  style_box.series_outer_padding({10, 50, 10, 20});
  style_box.series_bg_color({10, 50, 10, 20});
  style_box.series_align_max();

  datagui::Style style_text_input;
  style_text_input.font_size(40);

  auto timer = gui.variable<int>(0);
  using clock_t = std::chrono::high_resolution_clock;
  auto next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));

  auto timer_paused = gui.variable<bool>(false);

  while (gui.running()) {
    if (gui.begin()) {
      gui.style(style_root);
      if (gui.series_begin()) {
        gui.style(style_h1);
        gui.text_box("Welcome Screen!");

        auto name = gui.variable<std::string>("");

        gui.style(style_box);
        if (gui.series_begin()) {
          gui.text_box("Name: ");
          gui.style(style_text_input);
          gui.text_input(name);
          gui.series_end();
        }
        if (auto value = gui.checkbox()) {
          std::cout << "Checked: " << *value << std::endl;
        }

        gui.style(style_text);
        if (name->empty()) {
          gui.text_box("Hello... what is your name?");
        } else {
          gui.text_box("Hello " + *name);
        }

        gui.text_box("Timer: " + std::to_string(*timer));

        if (gui.button("Reset")) {
          timer.set(0);
          next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));
        }

        if (gui.series_begin()) {
          gui.checkbox(timer_paused);
          gui.text_box("Paused");
          gui.series_end();
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
