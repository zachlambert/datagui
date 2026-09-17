#pragma once

#include "datagui/gui.hpp"
#include "datagui/widget/plotter.hpp"

namespace dgui {

inline void plot(
    const std::function<void(Plotter&)> plot,
    const std::string& title = "Plot") {
  Gui gui;
  gui.open(title);
  while (gui.poll()) {
    auto& plotter = gui.plotter();
    plot(plotter);
    gui.end();
  }
}

} // namespace dgui
