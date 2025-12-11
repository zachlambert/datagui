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

        std::vector<Vec2> a, b, c, d, e, f;
        for (float x = 0; x < 2 * M_PI; x += 0.1) {
          a.push_back({t + x, std::sin(5 * t + x)});
          b.push_back({t + x, std::sin(5 * t + x + 1 * M_PIf / 3)});
          c.push_back({t + x, std::sin(5 * t + x + 2 * M_PIf / 3)});
          d.push_back({t + x, std::sin(5 * t + x + 3 * M_PIf / 3)});
          e.push_back({t + x, std::sin(5 * t + x + 4 * M_PIf / 3)});
          f.push_back({t + x, std::sin(5 * t + x + 5 * M_PIf / 3)});
        }

        plotter->title("Sine Waves");
        plotter->xlabel("Time");
        plotter->ylabel("Value");

        plotter->plot(a).line_solid(6).label("a");
        plotter->plot(b).label("b");
        plotter->plot(c).label("c");
        plotter->plot(d).label("d");
        plotter->plot(e).label("e");
        plotter->plot(f).label("f");
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
