#include "datagui/viewport/canvas2d.hpp"
#include "datagui/gui.hpp"

int main() {
  dgui::Gui gui;
  using dgui::Box2;
  using dgui::Color;
  using dgui::Vec2;

  Color color = Color::Red();
  float border_size = 0.05;
  float width = 1;
  Vec2 click_1;
  Vec2 click_2;

  while (gui.poll()) {
    gui.group();
    DGUI_SCOPE(gui);

    gui.text_box("Canvas");

    auto& canvas = gui.canvas2d(500, 200);
    {
      DGUI_SCOPE(gui);
      canvas.view_size(5, 2);
      canvas.bg_color(Color::Hsl(300, 0.4, 0.8));
      canvas.rect(Vec2(), 0, Vec2::uniform(width), color, border_size);
      canvas.text(std::to_string(width), Vec2(width / 2, width / 2));

      if ((click_1 - click_2).length() > 0.02) {
        canvas.capsule(click_1, click_2, 0.02, Color::Blue());
      }

      if (auto event = canvas.mouse_event()) {
        if (event->button == dgui::MouseButton::Left) {
          click_1 = event->press_position;
          click_2 = event->position;
        }
      }
    }

    gui.args().bg_color(Color::Hsl(220, 0.3, 0.8));
    gui.args().grid(-1, 2);
    if (gui.collapsable("Properties")) {
      DGUI_SCOPE(gui);

      gui.text_box("Color");
      gui.args().always();
      gui.color_picker_v(color);

      gui.text_box("Width");
      gui.args().always();
      gui.slider_v<float>(width, 0.1, 2);

      gui.text_box("Border size");
      gui.args().always();
      gui.slider_v<float>(border_size, 0, width / 2);
    }
  }
}
