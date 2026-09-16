#pragma once

#include "datagui/plot/heatmap_plot.hpp"
#include "datagui/plot/line_plot.hpp"
#include "datagui/viewport/viewport.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace dgui {

struct PlotterArgs {
  float tick_length = 5;
  float inner_padding = 5;
  float outer_padding = 10;
  float line_width = 2;
  Color tick_color = Color::Gray(0.2);
  float heatmap_scale_width = 16;
};

class Plotter : public Viewport {
public:
  LinePlot::Builder plot(const std::vector<Vec2>& points);
  LinePlot::Builder plot(std::vector<Vec2>&& points);

  LinePlot::Builder plot(
      const float* x,
      const float* y,
      std::size_t size,
      std::size_t stride = sizeof(float));
  LinePlot::Builder plot(
      const std::vector<float>& x,
      const std::vector<float>& y);

  LinePlot::Builder plot(
      const double* x,
      const double* y,
      std::size_t size,
      std::size_t stride = sizeof(double));
  LinePlot::Builder plot(
      const std::vector<double>& x,
      const std::vector<double>& y);

  LinePlot::Builder plot_function(
      const std::function<float(float)>& f,
      float x_min,
      float x_max,
      float x_resolution);
  LinePlot::Builder plot_function(
      const std::function<double(double)>& f,
      double x_min,
      double x_max,
      double x_resolution);

  HeatmapPlot::Builder heatmap(
      const Vec2& lower,
      const Vec2& upper,
      const std::function<float(const Vec2&)>& function,
      std::size_t width,
      std::size_t height);

  void title(const std::string& title) {
    title_ = title;
  }
  void xlabel(const std::string& xlabel) {
    xlabel_ = xlabel;
  }
  void ylabel(const std::string& ylabel) {
    ylabel_ = ylabel;
  }
  void xlimit(float lower, float upper) {
    xlimit_ = std::make_pair(lower, upper);
  }
  void ylimit(float lower, float upper) {
    ylimit_ = std::make_pair(lower, upper);
  }
  void undistorted() {
    undistorted_ = true;
  }

private:
  void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontRegistry>& font_registry) override;

  void begin() override;
  void draw(const Box2& viewport, DrawList& dl) override;

  void mouse_event(const MouseEvent& event) override;
  bool scroll_event(const ScrollEvent& event) override;

  struct Tick {
    float position;
    std::string label;
  };
  std::tuple<std::string, std::vector<Tick>> get_ticks(float min, float max);

  std::shared_ptr<Theme> theme;
  std::shared_ptr<FontRegistry> font_registry;

#if 0
  PlotterArgs args;
#endif
  std::vector<std::unique_ptr<Plot>> plots;
  std::size_t default_color_i = 0;

  bool mouse_down_valid = false;
  Vec2 mouse_down_pos;
  Box2 mouse_down_subview;
  Box2 subview = Box2(Vec2(), Vec2::ones());
  Box2 plot_area;

  std::string title_;
  std::string xlabel_;
  std::string ylabel_;
  std::optional<std::pair<float, float>> xlimit_;
  std::optional<std::pair<float, float>> ylimit_;
  bool undistorted_ = false;
};

}; // namespace dgui
