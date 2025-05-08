#include <chrono>
#include <datagui/gui.hpp>
#include <iostream>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  datagui::Style style_root;
  style_root.padding({40, 10});
  style_root.layout_direction(datagui::Direction::Vertical);
  style_root.layout_inner_padding(10);

  datagui::Style style_h1;
  style_h1.text_color(datagui::Color::Red());
  style_h1.font_size(40);

  datagui::Style style_text;
  style_text.font_size(30);

  datagui::Style style_horiz_expand;
  style_horiz_expand.layout_direction(datagui::Direction::Horizontal);
  style_horiz_expand.width(1.0_dynamic);
  style_horiz_expand.padding({10, 50, 10, 20});
  style_horiz_expand.bg_color({10, 50, 10, 20});
  style_horiz_expand.layout_alignment(datagui::Alignment::Bottom);
  style_horiz_expand.border_width({4, 12});
  style_horiz_expand.border_color(datagui::Color::Hsl(260, 0.4, 0.7));

  datagui::Style style_text_input;
  style_text_input.font_size(40);

  auto timer = gui.variable<int>(0);
  using clock_t = std::chrono::high_resolution_clock;
  auto next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));

  auto timer_paused = gui.variable<bool>(false);

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(style_root)) {
      gui.text_box("Welcome Screen!", style_h1);

      auto name = gui.variable<std::string>("");

      if (gui.horizontal_layout(style_horiz_expand)) {
        gui.text_box("Name: ", style_text);
        gui.text_input(name, style_text_input);
        gui.layout_end();
      }
      if (auto value = gui.checkbox()) {
        std::cout << "Checked: " << *value << std::endl;
      }

      if (name->empty()) {
        gui.text_box("Hello... what is your name?", style_text);
      } else {
        gui.text_box("Hello " + *name, style_text);
      }

      gui.text_box("Timer: " + std::to_string(*timer));

      if (gui.button("Reset")) {
        timer.set(0);
        next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));
      }

      if (gui.horizontal_layout()) {
        gui.checkbox(timer_paused);
        gui.text_box("Paused");
        gui.layout_end();
      }

      gui.layout_end();
    }
    gui.end();

    if (clock_t::now() > next_t) {
      if (!*timer_paused) {
        timer.set(*timer + 1);
      }
      next_t += clock_t::duration(std::chrono::seconds(1));
    }
  }
  return 0;
}
