#include "datagui/viewport/canvas2d.hpp"
#include "datagui/gui.hpp"

int main() {
  datagui::Gui gui;
  using datagui::Box2;
  using datagui::Color;
  using datagui::Vec2;

  while (gui.running()) {
    if (gui.group()) {
      gui.text_box("Canvas");

      auto color = gui.variable<Color>(Color::Red());
      auto border_size = gui.variable<float>(5);
      auto width = gui.variable<float>(100);

      if (auto canvas = gui.viewport<datagui::Canvas2d>(200, 200)) {
        canvas->bg_color(Color::Hsl(300, 0.4, 0.8));
        canvas->rect(Vec2(), 0, Vec2::uniform(*width), *color, *border_size);
        gui.end();
      }

      gui.args().bg_color(Color::Hsl(220, 0.3, 0.8));
      gui.args().grid(-1, 2);
      if (gui.collapsable("Properties")) {
        gui.text_box("Color");
        gui.args().always();
        gui.color_picker(color);

        gui.text_box("Width");
        gui.args().always();
        gui.slider<float>(10, 200, width);

        gui.text_box("Border size");
        gui.args().always();
        gui.slider<float>(0, *width / 2, border_size);

        gui.end();
      }

      gui.end();
    }
    gui.poll();
  }
}
