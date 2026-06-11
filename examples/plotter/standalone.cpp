#include "datagui/gui.hpp"
#include "datagui/viewport/plotter.hpp"
#include <cmath>

int main() {
  dgui::Gui gui;

  std::vector<double> xs;
  std::vector<double> ys;
  for (int i = 0; i <= 1000; i++) {
    double x = double(i) / 500;
    xs.push_back(x);
    ys.push_back(std::exp(-x) * std::cos(10 * M_PI * x));
  }

  while (gui.poll()) {
    auto& plotter = gui.plotter(400, 400);
    DGUI_SCOPE(gui);

    plotter.plot(xs, ys);
  }
}
