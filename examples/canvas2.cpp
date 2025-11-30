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
      if (auto canvas = gui.viewport<datagui::Canvas2>(300, 300)) {
        canvas->box(
            Box2(Vec2(100, 100), Vec2(200, 200)),
            Color::Red(0.8),
            20,
            10);
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
