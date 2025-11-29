#include "datagui/viewport/canvas2.hpp"
#include "datagui/gui.hpp"

int main() {
  datagui::Gui gui;
  using datagui::Box2;
  using datagui::Color;
  using datagui::Vec2;

  while (gui.running()) {
    if (auto canvas = gui.viewport<datagui::Canvas2>(500, 500)) {
      canvas->box(Box2(Vec2(0, 0), Vec2(500, 500)), Color::Red(0.5));
      gui.end();
    }
    gui.poll();
  }
}
