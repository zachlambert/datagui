#include "datagui/viewport/plotter.hpp"
#include "datagui/gui.hpp"
#include <chrono>
#include <cmath>

int main() {
  datagui::Gui gui;
  using datagui::Box2;
  using datagui::Color;
  using datagui::Vec2;

  auto t_start = std::chrono::high_resolution_clock::now();

  while (gui.running()) {
    if (gui.group()) {
      auto count = gui.variable<int>();
      gui.button("Increment", [=]() { count.mut()++; });
      if (auto plotter = gui.viewport<datagui::Plotter>(500, 500)) {
        gui.retrigger();

        float t = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::high_resolution_clock::now() - t_start)
                      .count() *
                  1e-9;

        std::vector<Vec2> a, b, c;
        for (float x = 0; x < 2 * M_PI; x += 0.1) {
          a.push_back({x, std::sin(5 * t + x)});
          b.push_back({x, std::sin(5 * t + x + 2 * M_PIf / 3)});
          c.push_back({x, std::sin(5 * t + x + 4 * M_PIf / 3)});
        }

        plotter->xlabel("Time");
        plotter->ylabel("Value");
        plotter->plot(a).line_solid(4);
        plotter->plot(b);
        plotter->plot(c);
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
