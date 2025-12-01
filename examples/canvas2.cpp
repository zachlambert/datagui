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

      auto color = gui.variable<Color>(Color::Red());
      auto border_size = gui.variable<float>(5);
      auto width = gui.variable<float>(100);
      auto radius = gui.variable<float>(10);

      if (auto canvas = gui.viewport<datagui::Canvas2>(200, 200)) {
        gui.depend_variable(color);
        gui.depend_variable(border_size);
        gui.depend_variable(width);
        gui.depend_variable(radius);

        Vec2 center = Vec2(100, 100);
        Vec2 lower = center - Vec2::uniform(*width / 2);
        Vec2 upper = center + Vec2::uniform(*width / 2);

        canvas->box(Box2(lower, upper), *color, *radius, *border_size);
        gui.end();
      }

      gui.args().bg_color(Color::Hsl(220, 0.3, 0.8));
      if (gui.section("Properties")) {
        gui.args().tight();
        if (gui.series()) {
          if (gui.labelled("Bg color")) {
            gui.args().always();
            gui.color_picker(color);
            gui.end();
          }
          if (gui.labelled("Width")) {
            gui.args().always();
            gui.slider<float>(0, 200, width);
            gui.end();
          }
          if (gui.labelled("Border size")) {
            gui.depend_variable(width);
            gui.args().always();
            gui.slider<float>(0, *width / 2, border_size);
            gui.end();
          }
          if (gui.labelled("Radius")) {
            gui.depend_variable(width);
            gui.args().always();
            gui.slider<float>(0, *width / 2, radius);
            gui.end();
          }
          gui.end();
        }
        gui.end();
      }

      gui.end();
    }
    gui.poll();
  }
}
