#include "datagui/viewport/plotter.hpp"
#include "datagui/gui.hpp"
#include <cmath>

int main() {
  datagui::Gui gui;
  using datagui::Box2;
  using datagui::Color;
  using datagui::Vec2;

  std::vector<Vec2> points;
  for (float x = 0; x < 2 * M_PI; x += 1e-2) {
    points.push_back({x, std::sin(x)});
  }

  while (gui.running()) {
    if (gui.group()) {
      if (auto plotter = gui.viewport<datagui::Plotter>(500, 500)) {
        plotter->xlabel("x");
        plotter->plot(points);
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
