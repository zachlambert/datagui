#include "datagui/viewport/canvas3d.hpp"
#include "datagui/gui.hpp"

int main() {
  datagui::Gui gui;
  using datagui::Color;
  using datagui::Rot3;
  using datagui::Vec3;

  while (gui.running()) {
    if (gui.group()) {
      if (auto canvas = gui.viewport<datagui::Canvas3d>(500, 500)) {
        canvas->grid(10, 10);
        canvas->box(Vec3(0, 0, 1), Rot3(), Vec3::uniform(2), Color::Blue());
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
