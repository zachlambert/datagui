#include <chrono>
#include <datagui/gui.hpp>

int main() {
  using namespace datagui::literals;

  datagui::Gui gui;

  auto style_root = [](datagui::VerticalLayoutStyle& style) {
    style.padding = {40, 10};
    style.inner_padding = 10;
    style.horizontal_alignment = datagui::AlignmentX::Center;
  };

  auto style_h1 = [](datagui::TextStyle& style) {
    style.text_color = datagui::Color::Red();
    style.font_size = 40;
  };

  auto style_text = [](datagui::TextStyle& style) { style.font_size = 30; };

  auto style_horiz_expand = [](datagui::HorizontalLayoutStyle& style) {
    style.length = 1.0_dynamic;
    style.width = _wrap;
    style.padding = {10, 50, 10, 20};
    style.bg_color = datagui::Color::Hsl(260, 0.4, 0.9);
    style.vertical_alignment = datagui::AlignmentY::Bottom;
    style.border_width = {4, 12};
    style.border_color = datagui::Color::Hsl(260, 0.4, 0.7);
    style.radius = 10;
  };

  auto style_text_input = [](datagui::TextInputStyle& style) {
    style.text_width = 1.0_dynamic;
    style.font_size = 40;
  };

  auto timer = gui.data<int>(0);
  using clock_t = std::chrono::high_resolution_clock;
  auto next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));

  while (gui.running()) {
    gui.begin();
    if (gui.vertical_layout(style_root)) {
      gui.text_box("Welcome Screen!", style_h1);

      auto name = gui.data<std::string>("");

      if (gui.horizontal_layout(style_horiz_expand)) {
        gui.text_box("Name: ", style_text);
        auto data = gui.text_input(style_text_input);
        if (data.modified()) {
          name.mut() = *data;
        }
        gui.layout_end();
      }

      if (name->empty()) {
        gui.text_box("Hello... what is your name?", style_text);
      } else {
        gui.text_box("Hello " + *name, style_text);
      }

      gui.text_box("Timer: " + std::to_string(*timer));

      if (gui.button("Reset")) {
        timer.mut() = 0;
        next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));
      }

      gui.layout_end();
    }
    gui.end();

    if (clock_t::now() > next_t) {
      timer.mut() += 1;
      next_t += clock_t::duration(std::chrono::seconds(1));
    }
  }
  return 0;
}
