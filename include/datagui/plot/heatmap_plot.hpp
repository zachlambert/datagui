#pragma once

#include "datagui/plot/plot.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/theme.hpp"
#include <functional>
#include <optional>

namespace dgui {

enum class HeatmapType { Viridis, Linear };

class HeatmapPlot : public Plot {
public:
  struct Args {
    std::string label;
    HeatmapType type;
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

  Box2 get_data_bounds() const override {
    return Box2(lower_, upper_);
  }
  void draw_data(const Box2& data_box, const Box2& draw_box, Scene2d& scene)
      const override;

  Vec2 legend_item_size() const override;
  void draw_legend_item(DrawList& dl, const Vec2& origin) const override;

  Builder builder() {
    return Builder(args_);
  }

private:
  Vec3 get_color(float s) const;

  std::shared_ptr<Theme> theme_;
  std::shared_ptr<FontRegistry> font_registry_;

  Vec2 lower_;
  Vec2 upper_;
  std::function<float(const Vec2&)> function_;
  size_t width_;
  size_t height_;

  Args args_;
};

} // namespace dgui
