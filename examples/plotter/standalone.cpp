#include "datagui/plot.hpp"
#include <cmath>

int main() {
  std::vector<double> xs;
  std::vector<double> ys;
  for (int i = 0; i <= 1000; i++) {
    double x = double(i) / 500;
    xs.push_back(x);
    ys.push_back(std::exp(-x) * std::cos(10 * M_PI * x));
  }

  dgui::plot([&](dgui::Plotter& plotter) {
    plotter.xlabel("x");
    plotter.ylabel("y");
    plotter.plot(xs, ys);
  });
}
