#pragma once

#include "datagui/gui.hpp"
#include "datagui/viewport/plotter.hpp"

namespace datagui {

inline void plot(
    const std::string& title,
    std::size_t width,
    std::size_t height,
    const std::function<void(Plotter&)> plot) {
  Gui gui(title);
  while (gui.running()) {
    if (auto plotter = gui.viewport<Plotter>(width, height)) {
      plot(*plotter);
      gui.end();
    }
    gui.poll();
  }
}

} // namespace datagui
