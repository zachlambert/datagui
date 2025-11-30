#include "datagui/viewport/canvas2.hpp"
#include "datagui/gui.hpp"

int main() {
  datagui::Gui gui;
  using datagui::Box2;
  using datagui::Color;
  using datagui::Vec2;

  while (gui.running()) {
    if (gui.series()) {
      gui.text_box("Canvas");

      auto color = gui.variable<Color>(Color::Black());
      if (gui.series()) {
        for (std::size_t i = 0; i < 3; i++) {
          gui.text_input("0", [i, color](const std::string& value) {
            try {
              float number = std::stof(value);
              color.mut().data[i] = number;
            } catch (const std::invalid_argument&) {
              // Do nothing
            }
          });
        }
        gui.end();
      }

      if (auto canvas = gui.viewport<datagui::Canvas2>(300, 300)) {
        gui.depend_variable(color);
        printf("%f, %f, %f\n", color->r, color->g, color->b);
        canvas->box(Box2(Vec2(100, 100), Vec2(200, 200)), *color, 20, 10);
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
