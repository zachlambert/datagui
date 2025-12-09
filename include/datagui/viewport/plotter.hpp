#pragma once

#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/shape_shader.hpp"
#include "datagui/visual/text_shader.hpp"
#include <vector>

namespace datagui {

extern std::vector<Color> default_plot_colors;

enum class PlotLineStyle { Solid, Dashed };
enum class PlotMarkerStyle { None, Circle };

struct PlotArgs {
  std::string label;
  Color color;
  PlotLineStyle line_style = PlotLineStyle::Solid;
  PlotMarkerStyle marker_style = PlotMarkerStyle::None;
  float line_width = 2;
  float marker_width = 5;
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
  PlotHandle& marker_circle() {
    args.marker_style = PlotMarkerStyle::Circle;
    return *this;
  }
  PlotHandle& line_dashed() {
    args.line_style = PlotLineStyle::Dashed;
    return *this;
  }

private:
  PlotHandle(PlotArgs& args) : args(args) {}

  PlotArgs& args;
  friend class Plotter;
};

struct PlotterArgs {
  float tick_length = 5;
  float padding = 5;
  float line_width = 2;
  Color tick_color = Color::Gray(0.8);
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

  void xlabel(const std::string& xlabel) {
    xlabel_ = xlabel;
  }

  void ylabel(const std::string& ylabel) {
    ylabel_ = ylabel;
  }

private:
  void begin() override;
  void end() override;
  void mouse_event(const MouseEvent& event) override;

  void impl_init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontManager>& fm) override;
  void impl_render() override;

  struct Tick {
    float position;
    float value;
  };
  std::vector<Tick> get_ticks(float min, float max);

  PlotterArgs args;

  Vec2 mouse_down_pos;
  Vec2 offset;

  struct PlotItem {
    PlotArgs args;
    std::vector<Vec2> points;
  };
  std::vector<PlotItem> plot_items;

  std::string xlabel_;
  std::string ylabel_;

  std::shared_ptr<Theme> theme;
  std::shared_ptr<FontManager> fm;
  ShapeShader shape_shader;
  TextShader text_shader;
};

}; // namespace datagui
