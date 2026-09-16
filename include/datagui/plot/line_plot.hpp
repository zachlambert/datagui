#pragma once

#include "datagui/plot/plot.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/theme.hpp"

namespace dgui {

enum class LineStyle { None, Solid, Dashed };
enum class MarkerStyle { None, Circle, Cross };

class LinePlot : public Plot {
  static constexpr float marker_default_width = 8;
  static constexpr float line_default_width = 2;

public:
  struct Args {
    std::string label;
    Color color = Color::Black();
    LineStyle line_style = LineStyle::Solid;
    MarkerStyle marker_style = MarkerStyle::None;
    float line_width = 2;
    float marker_width = 8;
  };

  class Builder {
  public:
    Builder& label(const std::string& label);
    Builder& color(const Color& color);
    Builder& marker_circle(float width = marker_default_width);
    Builder& marker_cross(float width = marker_default_width);
    Builder& no_line();
    Builder& line_solid(float width = line_default_width);
    Builder& line_dashed(float width = line_default_width);

  private:
    Builder(Args& args) : args(args) {}
    Args& args;
    friend class LinePlot;
  };

  LinePlot(
      std::shared_ptr<Theme> theme_,
      std::shared_ptr<FontRegistry> font_registry_,
      const std::vector<Vec2>& points);
  LinePlot(
      std::shared_ptr<Theme> theme_,
      std::shared_ptr<FontRegistry> font_registry_,
      std::vector<Vec2>&& points);

  void add_frame_components(PlotFrame& frame) override;
  void draw_frame_components(const PlotFrame& frame, DrawList& dl)
      const override;

  void draw_data(const PlotFrame& frame, Scene2d& scene) const override;

  Builder builder() {
    return Builder(args_);
  }

  static void reset_default_color_i() {
    default_color_i_ = 0;
  }

private:
  void init();

  std::shared_ptr<Theme> theme_;
  std::shared_ptr<FontRegistry> font_registry_;
  static size_t default_color_i_;
  Args args_;
  std::vector<Vec2> points_;
  Box2 data_bounds_;
  int legend_id_ = -1;
};

} // namespace dgui
