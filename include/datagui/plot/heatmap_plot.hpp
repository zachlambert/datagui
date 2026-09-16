#pragma once

#include "datagui/plot/plot.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/render/lookup/gradient_map.hpp"
#include "datagui/theme.hpp"
#include <functional>
#include <optional>

namespace dgui {

class HeatmapPlot : public Plot {
public:
  struct Args {
    std::string label;
    GradientMap gradient_map = GradientMap::viridis();
    Vec3 linear_min;
    Vec3 linear_max;
    std::optional<float> min_value;
    std::optional<float> max_value;
  };

  class Builder {
  public:
    Builder& label(const std::string& label);
    Builder& viridis();
    Builder& linear(const Color& min, const Color& max);
    Builder& min_value(float value);
    Builder& max_value(float value);

  private:
    Builder(Args& args) : args(args) {}

    Args& args;
    friend class HeatmapPlot;
  };

  HeatmapPlot(
      std::shared_ptr<Theme> theme,
      std::shared_ptr<FontRegistry> font_registry,
      const Vec2& lower,
      const Vec2& upper,
      const std::function<float(const Vec2&)>& function,
      size_t width,
      size_t height);

  void add_frame_components(PlotFrame& frame) override;
  void draw_frame_components(const PlotFrame& frame, DrawList& dl)
      const override;

  void draw_data(const PlotFrame& frame, Scene2d& scene) const override;

  Builder builder() {
    return Builder(args_);
  }

private:
  std::shared_ptr<Theme> theme_;
  std::shared_ptr<FontRegistry> font_registry_;

  Vec2 lower_;
  Vec2 upper_;
  std::function<float(const Vec2&)> function_;
  size_t width_;
  size_t height_;
  float data_min_ = 0;
  float data_max_ = 0;
  int icon_index_ = -1;

  Args args_;
};

} // namespace dgui
