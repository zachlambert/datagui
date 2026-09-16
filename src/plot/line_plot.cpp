#include "datagui/plot/line_plot.hpp"
#include "datagui/render/lookup/matplotlib_colors.hpp"

namespace dgui {

static constexpr float dashed_segment_length = 20;

size_t LinePlot::default_color_i_ = 0;

LinePlot::Builder& LinePlot::Builder::label(const std::string& label) {
  args.label = label;
  return *this;
}

LinePlot::Builder& LinePlot::Builder::color(const Color& color) {
  args.color = color;
  return *this;
}

LinePlot::Builder& LinePlot::Builder::marker_circle(float width) {
  args.marker_style = MarkerStyle::Circle;
  args.marker_width = width;
  return *this;
}

LinePlot::Builder& LinePlot::Builder::marker_cross(float width) {
  args.marker_style = MarkerStyle::Cross;
  args.marker_width = width;
  return *this;
}

LinePlot::Builder& LinePlot::Builder::no_line() {
  args.line_style = LineStyle::None;
  return *this;
}

LinePlot::Builder& LinePlot::Builder::line_solid(float width) {
  args.line_style = LineStyle::Solid;
  args.line_width = width;
  return *this;
}

LinePlot::Builder& LinePlot::Builder::line_dashed(float width) {
  args.line_style = LineStyle::Dashed;
  args.line_width = width;
  return *this;
}

LinePlot::LinePlot(
    std::shared_ptr<Theme> theme,
    std::shared_ptr<FontRegistry> font_registry,
    const std::vector<Vec2>& points) :
    theme_(theme), font_registry_(font_registry), points_(points) {
  init();
}
LinePlot::LinePlot(
    std::shared_ptr<Theme> theme,
    std::shared_ptr<FontRegistry> font_registry,
    std::vector<Vec2>&& points) :
    theme_(theme), font_registry_(font_registry), points_(std::move(points)) {
  init();
}

void LinePlot::add_frame_components(PlotFrame& frame) {
  frame.add_data(data_bounds_.lower, data_bounds_.upper);
  if (!args_.label.empty()) {
    legend_id_ = frame.add_legend_item(args_.label);
  }
}

void LinePlot::draw_frame_components(const PlotFrame& frame, DrawList& dl)
    const {
  if (legend_id_ < 0) {
    return;
  }
  const Box2& box = frame.legend_icon_box(legend_id_);
  switch (args_.line_style) {
    case LineStyle::None:
      break;
    case LineStyle::Solid:
      dl.draw_line(
          box.center_left(),
          box.center_right(),
          args_.line_width,
          args_.color);
      break;
    case LineStyle::Dashed: {
      const float line_length = box.size_x();
      const Vec2 origin = box.center_left();
      float s = -line_length / 2;
      while (s < line_length / 2) {
        const int index = std::round(s / dashed_segment_length);
        if (index % 2 == 0) {
          double s2 = std::min(s + dashed_segment_length, line_length / 2);
          dl.draw_line(
              origin + Vec2(s, 0),
              origin + Vec2(s2, 0),
              args_.line_width,
              args_.color);
        }
        s += dashed_segment_length;
      }
      break;
    }
  }

  const Vec2 marker_pos = box.center();
  switch (args_.marker_style) {
    case MarkerStyle::Circle:
      dl.draw_circle(marker_pos, args_.marker_width / 2, args_.color);
      break;
    case MarkerStyle::Cross: {
      Vec2 delta_up(args_.marker_width / 2, args_.marker_width / 2);
      Vec2 delta_down(args_.marker_width / 2, -args_.marker_width / 2);
      dl.draw_line(
          marker_pos - delta_up,
          marker_pos + delta_up,
          args_.marker_width * 0.3,
          args_.color);
      dl.draw_line(
          marker_pos - delta_down,
          marker_pos + delta_down,
          args_.marker_width * 0.3,
          args_.color);
    } break;
    default:
      break;
  }
}

void LinePlot::draw_data(const PlotFrame& frame, Scene2d& scene) const {
  auto plot_marker = [&](const Vec2& point) {
    Vec2 draw_point = remap(point, frame.data_area(), frame.plot_area());
    switch (args_.marker_style) {
      case MarkerStyle::Circle:
        scene.draw_circle(
            draw_point,
            args_.marker_width / 2,
            args_.color,
            0,
            Color::Black());
        break;
      case MarkerStyle::Cross: {
        Vec2 delta_up(args_.marker_width / 2, args_.marker_width / 2);
        Vec2 delta_down(args_.marker_width / 2, -args_.marker_width / 2);
        scene.draw_line(
            draw_point - delta_up,
            draw_point + delta_up,
            args_.marker_width * 0.3,
            args_.color);
        scene.draw_line(
            draw_point - delta_down,
            draw_point + delta_down,
            args_.marker_width * 0.3,
            args_.color);
      } break;
      default:
        break;
    }
  };

  auto plot_line = [&](const Vec2& a, const Vec2& b, float length) {
    Vec2 draw_a = remap(a, frame.data_area(), frame.plot_area());
    Vec2 draw_b = remap(b, frame.data_area(), frame.plot_area());
    float ab_length = (draw_a - draw_b).length();
    Vec2 dir = (draw_b - draw_a) / ab_length;
    switch (args_.line_style) {
      case LineStyle::Solid:
        scene.draw_line(draw_a, draw_b, args_.line_width, args_.color);
        break;
      case LineStyle::Dashed: {
        float s1 = -std::fmod(length, dashed_segment_length);
        while (s1 < ab_length) {
          float s2 = s1 + dashed_segment_length;
          int i = (length + s1) / dashed_segment_length;
          if (i % 2 == 0) {
            scene.draw_line(
                draw_a + std::max(s1, 0.f) * dir,
                draw_a + std::min(s2, ab_length) * dir,
                args_.line_width,
                args_.color,
                false);
          }
          s1 = s2;
        }
        break;
      }
      default:
        break;
    }
    return ab_length;
  };

  float length = 0;
  for (std::size_t i = 0; i + 1 < points_.size(); i++) {
    const Vec2& a = points_[i];
    const Vec2& b = points_[i + 1];
    float line_length = plot_line(a, b, length);
    plot_marker(a);
    length += line_length;
  }
  if (!points_.empty()) {
    plot_marker(points_.back());
  }
}

void LinePlot::init() {
  args_.color = matplotlib_colors[default_color_i_];
  default_color_i_ = (default_color_i_ + 1) % matplotlib_colors.size();

  if (points_.empty()) {
    data_bounds_ = Box2(Vec2(), Vec2::ones());
    return;
  }
  data_bounds_.lower = Vec2::uniform(std::numeric_limits<float>::max());
  data_bounds_.upper = Vec2::uniform(-std::numeric_limits<float>::max());
  for (const auto& point : points_) {
    data_bounds_.lower = minimum(data_bounds_.lower, point);
    data_bounds_.upper = maximum(data_bounds_.upper, point);
  }
}

} // namespace dgui
