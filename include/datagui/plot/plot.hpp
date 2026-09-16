#pragma once

#include "datagui/geometry/box.hpp"
#include "datagui/plot/plot_frame.hpp"
#include "datagui/render/state/draw_list.hpp"
#include "datagui/render/state/scene_2d.hpp"

namespace dgui {

class Plot {
public:
  virtual void add_frame_components(PlotFrame& frame) const = 0;
  virtual void draw_frame_components(const PlotFrame& frame, DrawList& dl)
      const = 0;

  virtual void draw_data(
      const PlotFrame& frame,
      Scene2d& scene) const = 0;

  #if 0
  virtual Box2 get_data_bounds() const = 0;
  virtual Vec2 legend_item_size() const = 0;
  virtual void draw_legend_item(DrawList& dl, const Vec2& origin) const = 0;
  #endif
};

} // namespace dgui
