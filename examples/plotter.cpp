#include "datagui/viewport/plotter.hpp"
#include "datagui/gui.hpp"
#include <chrono>
#include <cmath>

int main() {
  datagui::Gui gui;
  using datagui::Box2;
  using datagui::Color;
  using datagui::Vec2;

  bool paused = false;
  auto prev = std::chrono::high_resolution_clock::now();
  float t = 0;
  float freq = 5;
  float T = 1;
  std::size_t N = 20;

  while (gui.running()) {
    if (gui.group()) {
      gui.args().grid(-1, 2);
      if (gui.group()) {
        gui.text_box("Paused");
        gui.checkbox(paused);

        gui.text_box("Frequency");
        gui.args().always();
        gui.slider(freq, 1.f, 10.f);

        gui.text_box("Time horizon");
        gui.args().always();
        gui.slider(T, 0.1f, 10.f);

        gui.end();
      }
      if (auto plotter = gui.viewport<datagui::Plotter>(500, 500)) {
        gui.retrigger();

        auto now = std::chrono::high_resolution_clock::now();
        float dt =
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev)
                .count() *
            1e-9;
        prev = now;
        if (!paused) {
          t += dt;
        }

        std::vector<Vec2> a, b, c, d, e, f;
        for (float dt = 0; dt < T; dt += T / N) {
          a.push_back({t + dt, std::sin(freq * (t + dt))});
          b.push_back({t + dt, std::sin(freq * (t + dt) + 1 * M_PIf / 3)});
          c.push_back({t + dt, std::sin(freq * (t + dt) + 2 * M_PIf / 3)});
          d.push_back({t + dt, std::sin(freq * (t + dt) + 3 * M_PIf / 3)});
          e.push_back({t + dt, std::sin(freq * (t + dt) + 4 * M_PIf / 3)});
          f.push_back({t + dt, std::sin(freq * (t + dt) + 5 * M_PIf / 3)});
        }

        plotter->title("Sine Waves");
        plotter->xlabel("Time");
        plotter->ylabel("Value");

        plotter->plot(a).line_solid(6).label("a");
        plotter->plot(b).label("b").marker_circle();
        plotter->plot(c).label("c").marker_cross();
        plotter->plot(d).label("d").line_dashed();
        plotter->plot(e).label("e").line_dashed();
        plotter->plot(f).label("f").no_line().marker_cross();
        gui.end();
      }
      gui.end();
    }
    gui.poll();
  }
}
