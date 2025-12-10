#include "datagui/viewport/plotter.hpp"
#include "datagui/gui.hpp"
#include <cmath>

int main() {
  datagui::Gui gui;
  using datagui::Box2;
  using datagui::Color;
  using datagui::Vec2;

  std::vector<Vec2> a, b, c;
  for (float x = 0; x < 2 * M_PI; x += 0.1) {
    a.push_back({x, std::sin(x)});
    b.push_back({x, std::sin(x + 0.5f)});
    c.push_back({x, std::sin(x + 1.0f)});
  }

  while (gui.running()) {
    if (gui.group()) {
      auto count = gui.variable<int>();
      gui.button("Increment", [=]() { count.mut()++; });
      if (auto plotter = gui.viewport<datagui::Plotter>(500, 500)) {
        plotter->xlabel(std::to_string(*count));
        plotter->plot(a).marker_circle();
        plotter->plot(b);
        plotter->plot(c);
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
