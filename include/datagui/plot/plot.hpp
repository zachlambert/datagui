#pragma once

#include "datagui/plot/plot_frame.hpp"
#include "datagui/render/state/draw_list.hpp"
#include "datagui/render/state/scene_2d.hpp"

namespace dgui {

class Plot {
public:
  virtual void add_frame_components(PlotFrame& frame) = 0;
  virtual void draw_frame_components(const PlotFrame& frame, DrawList& dl)
      const = 0;

  virtual void draw_data(
      const PlotFrame& frame,
      Scene2d& scene) const = 0;
};

} // namespace dgui
