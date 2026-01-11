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
      auto border_size = gui.variable<float>(0.05);
      auto width = gui.variable<float>(1);
      auto click_1 = gui.variable<Vec2>();
      auto click_2 = gui.variable<Vec2>();

      if (auto canvas = gui.viewport<datagui::Canvas2d>(500, 200)) {
        canvas->view_size(5, 2);
        canvas->bg_color(Color::Hsl(300, 0.4, 0.8));
        canvas->rect(Vec2(), 0, Vec2::uniform(*width), *color, *border_size);
        canvas->text(std::to_string(*width), Vec2(*width / 2, *width / 2));

        if ((*click_1 - *click_2).length() > 0.02) {
          canvas->capsule(*click_1, *click_2, 0.02, Color::Blue());
        }

        canvas->click_callback([=](const datagui::MouseEvent& event) {
          if (event.button != datagui::MouseButton::Left) {
            return;
          }
          click_1.set(event.press_position);
          click_2.set(event.position);
        });
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
        gui.slider<float>(0.1, 2, width);

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
