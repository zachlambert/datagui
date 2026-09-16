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

  Box2 get_data_bounds() const override {
    return data_bounds_;
  }
  void draw_data(const Box2& data_box, const Box2& draw_box, Scene2d& scene)
      const override;

  Vec2 legend_item_size() const override;
  void draw_legend_item(DrawList& dl, const Vec2& origin) const override;

  Builder builder() {
    return Builder(args_);
  }

private:
  void init();

  std::shared_ptr<Theme> theme_;
  std::shared_ptr<FontRegistry> font_registry_;
  size_t default_color_i_ = 0;
  Args args_;
  std::vector<Vec2> points_;
  Box2 data_bounds_;
};

} // namespace dgui
