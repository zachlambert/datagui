#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/image_shader.hpp"
#include "datagui/visual/shape_2d_shader.hpp"
#include "datagui/visual/text_2d_shader.hpp"
#include <functional>
#include <vector>

namespace datagui {

extern std::vector<Color> default_plot_colors;

enum class PlotLineStyle { None, Solid, Dashed };
enum class PlotMarkerStyle { None, Circle, Cross };

struct PlotArgs {
  std::string label;
  Color color = Color::Black();
  PlotLineStyle line_style = PlotLineStyle::Solid;
  PlotMarkerStyle marker_style = PlotMarkerStyle::None;
  float line_width = 2;
  float marker_width = 8;
};

class PlotHandle {
public:
  PlotHandle& label(const std::string& label) {
    args.label = label;
    return *this;
  }
  PlotHandle& color(const Color& color) {
    args.color = color;
    return *this;
  }
  PlotHandle& marker_circle(float width = 8) {
    args.marker_style = PlotMarkerStyle::Circle;
    args.marker_width = width;
    return *this;
  }
  PlotHandle& marker_cross(float width = 8) {
    args.marker_style = PlotMarkerStyle::Cross;
    args.marker_width = width;
    return *this;
  }
  PlotHandle& no_line() {
    args.line_style = PlotLineStyle::None;
    return *this;
  }
  PlotHandle& line_solid(float width = 2) {
    args.line_style = PlotLineStyle::Solid;
    args.line_width = width;
    return *this;
  }
  PlotHandle& line_dashed(float width = 2) {
    args.line_style = PlotLineStyle::Dashed;
    args.line_width = width;
    return *this;
  }

private:
  PlotHandle(PlotArgs& args) : args(args) {}

  PlotArgs& args;
  friend class Plotter;
};

enum class HeatmapType { Viridis, Linear };

struct HeatmapArgs {
  HeatmapType type;
  Vec3 linear_min;
  Vec3 linear_max;
  std::optional<float> min_value;
  std::optional<float> max_value;
};

class HeatmapHandle {
public:
  HeatmapHandle& viridis() {
    args.type = HeatmapType::Viridis;
    return *this;
  }
  HeatmapHandle& linear(const Color& min, const Color& max) {
    args.type = HeatmapType::Linear;
    args.linear_min = {min.r, min.g, min.b};
    args.linear_max = {max.r, max.g, max.b};
    return *this;
  }
  HeatmapHandle& min_value(float value) {
    args.min_value = value;
    return *this;
  }
  HeatmapHandle& max_value(float value) {
    args.max_value = value;
    return *this;
  }

private:
  HeatmapHandle(HeatmapArgs& args) : args(args) {}

  HeatmapArgs& args;
  friend class Plotter;
};

struct PlotterArgs {
  float tick_length = 5;
  float inner_padding = 5;
  float outer_padding = 10;
  float line_width = 2;
  Color tick_color = Color::Gray(0.2);
};

class Plotter : public Viewport {
public:
  PlotHandle plot(const std::vector<Vec2>& points);
  PlotHandle plot(std::vector<Vec2>&& points);

  PlotHandle plot(
      const float* x,
      const float* y,
      std::size_t size,
      std::size_t stride = 1);
  PlotHandle plot(const std::vector<float>& x, const std::vector<float>& y) {
    return plot(x.data(), y.data(), 1);
  }

  PlotHandle plot(
      const double* x,
      const double* y,
      std::size_t size,
      std::size_t stride = 1);
  PlotHandle plot(const std::vector<double>& x, const std::vector<double>& y) {
    return plot(x.data(), y.data(), 1);
  }

  HeatmapHandle heatmap(
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

private:
  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void begin() override;
  void end() override;
  void mouse_event(const Vec2& size, const MouseEvent& event) override;
  bool scroll_event(const Vec2& size, const ScrollEvent& event) override;
  void queue_commands();
  void redraw();

  struct Tick {
    float position;
    std::string label;
  };
  std::tuple<std::string, std::vector<Tick>> get_ticks(float min, float max);

  PlotterArgs args;

  Vec2 mouse_down_pos;
  Box2 mouse_down_subview;
  Box2 subview = Box2(Vec2(), Vec2::ones());
  Box2 prev_plot_area =
      Box2(Vec2(), Vec2::ones()); // Used for mouse event handling

  struct PlotItem {
    PlotArgs args;
    std::vector<Vec2> points;
  };
  std::vector<PlotItem> plot_items;

  struct HeatmapItem {
    HeatmapArgs args;
    Box2 bounds;
    std::function<float(const Vec2&)> function;
    std::size_t width;
    std::size_t height;
    mutable Image image;
  };
  std::vector<HeatmapItem> heatmap_items;

  std::string title_;
  std::string xlabel_;
  std::string ylabel_;

  std::size_t default_color_i = 0;
  std::shared_ptr<Theme> theme;
  std::shared_ptr<FontManager> fm;

  Shape2dShader fixed_shape_shader;
  Text2dShader fixed_text_shader;
  Shape2dShader plot_shape_shader;
  ImageShader plot_image_shader;
};

}; // namespace datagui
