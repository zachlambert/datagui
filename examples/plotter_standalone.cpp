#include "datagui/gui.hpp"
#include "datagui/viewport/plotter.hpp"
#include <cmath>

int main() {
  datagui::Gui gui;

  std::vector<double> xs;
  std::vector<double> ys;
  for (int i = 0; i <= 1000; i++) {
    double x = double(i) / 500;
    xs.push_back(x);
    ys.push_back(std::exp(-x) * std::cos(10 * M_PI * x));
  }

  while (gui.running()) {
    if (auto plotter = gui.viewport<datagui::Plotter>(400, 400)) {
      plotter->plot(xs, ys);
      gui.end();
    }
    gui.poll();
  }
}
