#include "datagui/viewport/canvas3d.hpp"
#include "datagui/gui.hpp"

int main() {
  datagui::Gui gui;
  using datagui::Color;
  using datagui::Euler;
  using datagui::Rot3;
  using datagui::Vec3;

  while (gui.running()) {
    if (gui.group()) {
      if (auto canvas = gui.viewport<datagui::Canvas3d>(512, 512)) {
        canvas->grid(10, 10);
        canvas
            ->box(Vec3(0, 0, 1), Rot3(), Vec3::uniform(2), Color(1, 0, 0, 0.8));
        canvas->box(
            Vec3(3, 0, 1),
            Rot3(Euler(M_PI / 4, 0, 0)),
            Vec3::uniform(2),
            Color(0, 1, 0, 0.5));
        canvas->box(
            Vec3(0, 3, 1),
            Rot3(Euler(0, M_PI / 4, 0)),
            Vec3::uniform(2),
            Color(0, 0, 1, 0.2));

        canvas->cylinder(
            Vec3(0, -3, 0),
            Vec3(std::sin(M_PI / 4), 0, std::cos(M_PI / 4)),
            0.5,
            5,
            Color::Hsl(50, 1, 0.5));

        canvas->sphere(Vec3(-3, 0, 1), 1, Color::Hsl(300, 1, 0.5));

        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
