#include "datagui/widget/canvas2d.hpp"
#include "datagui/gui.hpp"

struct Circle {
  double x;
  double y;
  float radius;
};
namespace dgui {
template <>
struct DrawArgs<Circle> {
  dgui::Color color;
  float border_width = 0;
  dgui::Color border_color = Color::Black();
};
void draw(
    Canvas2d& canvas,
    const Circle& circle,
    const DrawArgs<Circle>& args) {
  canvas.circle(
      Vec2{static_cast<float>(circle.x), static_cast<float>(circle.y)},
      static_cast<float>(circle.radius),
      args.color,
      args.border_width,
      args.border_color);
}
} // namespace dgui

int main() {
  dgui::Gui gui;
  gui.open();
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

    auto& canvas = gui.canvas2d();
    {
      DGUI_SCOPE(gui);
      canvas.default_view_width(5);
      canvas.bg_color(Color::Hsl(300, 0.4, 0.8));
      canvas.object(Circle{0.0, 0.0, 1.0}, {dgui::Color::Blue(), 0.02f, Color::White()});
      canvas.rect(Vec2(), 0, Vec2::uniform(width), color, border_size);
      canvas.label(std::to_string(width), Vec2(width / 2, width / 2));

      if ((click_1 - click_2).length() > 0.02) {
        canvas.capsule(click_1, click_2, 0.02, Color::Blue());
      }

      if (auto event = canvas.mouse_event()) {
        if (event->button == dgui::MouseButton::Right) {
          click_1 = event->press_position;
          click_2 = event->position;
        }
      }
    }
  }
}
