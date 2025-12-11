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
  Color color = Color::Black();
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
  PlotHandle& marker_circle(float width = 5) {
    args.marker_style = PlotMarkerStyle::Circle;
    args.marker_width = width;
    return *this;
  }
  PlotHandle& line_solid(float width) {
    args.line_style = PlotLineStyle::Solid;
    args.line_width = width;
    return *this;
  }
  PlotHandle& line_dashed(float width) {
    args.line_style = PlotLineStyle::Dashed;
    args.line_width = width;
    return *this;
  }

private:
  PlotHandle(PlotArgs& args) : args(args) {}

  PlotArgs& args;
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
  void render_content();

  struct Tick {
    float position;
    std::string label;
  };
  std::tuple<std::string, std::vector<Tick>> get_ticks(float min, float max);

  PlotterArgs args;

  Vec2 mouse_down_pos;
  Box2 mouse_down_subview;
  Box2 subview = Box2(Vec2(), Vec2::ones());

  struct PlotItem {
    PlotArgs args;
    std::vector<Vec2> points;
  };
  std::vector<PlotItem> plot_items;

  std::string title_;
  std::string xlabel_;
  std::string ylabel_;

  std::size_t default_color_i = 0;
  std::shared_ptr<Theme> theme;
  std::shared_ptr<FontManager> fm;
  ShapeShader shape_shader;
  TextShader text_shader;
};

}; // namespace datagui
