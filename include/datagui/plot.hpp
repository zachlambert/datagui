#pragma once

#include "datagui/gui.hpp"
#include "datagui/viewport/plotter.hpp"

namespace dgui {

inline void plot(
    const std::string& title,
    std::size_t width,
    std::size_t height,
    const std::function<void(Plotter&)> plot) {
  Gui gui;
  gui.open(title);
  while (gui.running()) {
    if (auto plotter = gui.plotter(width, height)) {
      plot(*plotter);
      gui.end();
    }
    gui.poll();
  }
}

} // namespace dgui
