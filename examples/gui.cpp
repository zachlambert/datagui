#include <chrono>
#include <datagui/gui.hpp>

int main() {
  datagui::Gui gui;

  auto style_root = [](datagui::LinearLayoutStyle& style) {
    style.padding = 10;
    style.inner_padding = 10;
  };

  auto style_h1 = [](datagui::TextStyle& style) {
    style.text_color = datagui::Color::Red();
    style.font_size = 40;
  };

  auto style_text = [](datagui::TextStyle& style) { style.font_size = 30; };

  auto style_horiz_expand = [](datagui::LinearLayoutStyle& style) {
    style.direction = datagui::LinearLayoutDirection::Horizontal;
    style.length = -1;
    style.width = 0;
    style.padding = 20;
  };

  auto style_text_input_expand = [](datagui::TextInputStyle& style) {
    style.max_width = -1;
  };

  auto timer = gui.data<int>(0);
  using clock_t = std::chrono::high_resolution_clock;
  auto next_t = clock_t::now() + clock_t::duration(std::chrono::seconds(1));

  while (gui.running()) {
    gui.begin();
    if (gui.linear_layout(style_root)) {
      gui.text("Welcome Screen!", style_h1);

      auto name = gui.data<std::string>("");

      if (gui.linear_layout(style_horiz_expand)) {
        gui.text("Name: ", style_text);
        auto data = gui.text_input(style_text_input_expand);
        if (data.modified()) {
          name.mut() = *data;
        }
        gui.container_end();
      }

      if (name->empty()) {
        gui.text("Hello... what is your name?", style_text);
      } else {
        gui.text("Hello " + *name, style_text);
      }

      gui.text("Timer: " + std::to_string(*timer));
      gui.container_end();
    }
    gui.end();

    if (clock_t::now() > next_t) {
      timer.mut() += 1;
      next_t += clock_t::duration(std::chrono::seconds(1));
    }
  }
  return 0;
}
