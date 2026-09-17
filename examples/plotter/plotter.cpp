#include "datagui/widget/plotter.hpp"
#include "datagui/gui.hpp"
#include <chrono>
#include <cmath>

int main() {
  dgui::Gui gui;
  gui.open();
  using dgui::Box2;
  using dgui::Color;
  using dgui::Vec2;

  bool paused = false;
  auto prev = std::chrono::high_resolution_clock::now();
  float t = 0;
  float freq = 5;
  float T = 1;
  std::size_t N = 100;

  float a = 1;
  float b = 1;

  while (gui.poll()) {
    gui.vsplit(0.5);
    DGUI_SCOPE(gui);

    gui.group();
    {
      DGUI_SCOPE(gui);

      auto& plotter = gui.plotter();
      {
        DGUI_SCOPE(gui);

        auto now = std::chrono::high_resolution_clock::now();
        float dt =
            std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev)
                .count() *
            1e-9;
        prev = now;
        if (!paused) {
          t += dt;
        }

        auto func = [](float freq, float t, float phase) {
          return std::max(0.f, std::sin(freq * t + phase));
        };
        static constexpr size_t plot_n = 6;
        std::array<std::vector<Vec2>, plot_n> plots;
        for (float dt = 0; dt < T; dt += T / N) {
          for (size_t i = 0; i < plot_n; i++) {
            plots[i].push_back({t + dt, func(freq, t + dt, i * M_PIf / 3)});
          }
        }

        plotter.title("Sine Waves");
        plotter.xlabel("Time");
        plotter.ylabel("Value");
        plotter.ylimit(-0.1, 1.1);

        plotter.plot(plots[0]).line_solid(6).label("a");
        plotter.plot(plots[1]).label("b").marker_circle();
        plotter.plot(plots[2]).label("c").marker_cross();
        plotter.plot(plots[3]).label("d").line_dashed();
        plotter.plot(plots[4]).label("e").line_dashed().marker_circle();
        plotter.plot(plots[5]).label("f").no_line().marker_cross();
      }

      gui.args().grid(-1, 2);
      gui.group();
      {
        DGUI_SCOPE(gui);
        gui.text_box("Paused");
        gui.checkbox_v(paused);

        gui.text_box("Frequency");
        gui.args().always();
        gui.slider_v(freq, 1.f, 10.f);

        gui.text_box("Time horizon");
        gui.args().always();
        gui.slider_v(T, 0.1f, 10.f);
      }
    }

    gui.group();
    {
      DGUI_SCOPE(gui);

      auto& plotter = gui.plotter();
      {
        DGUI_SCOPE(gui);
        auto f = [&](const Vec2& pos) {
          return std::exp(-pos.x * a) * std::exp(pos.y) *
                 std::sin(b * 2 * M_PIf * pos.y);
        };
        plotter.title("exp(-ax) * exp(y) * sin(2{pi}by)");
        plotter.heatmap(Vec2(0, -1), Vec2(2, 1), f, 100, 100);
      }

      gui.args().grid(-1, 2);
      gui.group();
      {
        DGUI_SCOPE(gui);

        gui.text_box("a");
        gui.args().always();
        gui.slider_v(a, 0.2f, 2.f);

        gui.text_box("b");
        gui.args().always();
        gui.slider_v(b, 0.2f, 2.f);
      }
    }
  }
}
