#include "datagui/plot/line_plot.hpp"
#include "datagui/render/lookup/matplotlib_colors.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

namespace dgui {

static constexpr float dashed_segment_length = 20;
static constexpr float dashed_segment_length_legend = 5;

namespace {

#if 0
// Clips the segment a -> b against the box, returning the parametric range
// [t_min, t_max] of the portion inside it, or false if it lies entirely
// outside
bool clip_segment(
    const Box2& box,
    const Vec2& a,
    const Vec2& b,
    float& t_min,
    float& t_max) {
  t_min = 0;
  t_max = 1;
  const Vec2 delta = b - a;
  for (std::size_t axis = 0; axis < 2; axis++) {
    if (delta(axis) == 0) {
      // Parallel to this axis, so it is either entirely within the box's
      // extent along it, or entirely outside
      if (a(axis) < box.lower(axis) || a(axis) > box.upper(axis)) {
        return false;
      }
      continue;
    }
    float t_lower = (box.lower(axis) - a(axis)) / delta(axis);
    float t_upper = (box.upper(axis) - a(axis)) / delta(axis);
    if (t_lower > t_upper) {
      std::swap(t_lower, t_upper);
    }
    t_min = std::max(t_min, t_lower);
    t_max = std::min(t_max, t_upper);
    if (t_min > t_max) {
      return false;
    }
  }
  return true;
}
#endif

} // namespace

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
      // The icon is much shorter than a plotted line, so shrink the segments
      // so that a few dashes are visible within it
      const float line_length = box.size_x();
      if (line_length <= 0) {
        break;
      }
      const float segment_length = std::min(
          dashed_segment_length,
          line_length / dashed_segment_length_legend);
      const Vec2 origin = box.center_left();
      float s = 0;
      while (s < line_length) {
        const float s2 = std::min(s + segment_length, line_length);
        dl.draw_line(
            origin + Vec2(s, 0),
            origin + Vec2(s2, 0),
            args_.line_width,
            args_.color,
            false);
        s += 2 * segment_length;
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
  // Used to exclude markers outside of this area
  // Expand by the marker width
  const Box2 marker_scene_area =
      frame.scene_area().from_expand(args_.marker_width);

  auto plot_marker = [&](const Vec2& point) {
    Vec2 draw_point = remap(point, frame.data_window(), frame.scene_area());
    if (!marker_scene_area.contains(draw_point)) {
      return;
    }
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

  switch (args_.line_style) {
    case LineStyle::Solid: {
      for (std::size_t i = 0; i + 1 < points_.size(); i++) {
        const Vec2 a =
            remap(points_[i], frame.data_window(), frame.scene_area());
        const Vec2 b =
            remap(points_[i + 1], frame.data_window(), frame.scene_area());
        if (!frame.scene_area().intersects_segment(a, b)) {
          continue;
        }
        scene.draw_line(a, b, args_.line_width, args_.color);
      }
      break;
    }
    case LineStyle::Dashed: {
      float phase = 0;
      const float period = 2 * dashed_segment_length;
      for (std::size_t i = 0; i + 1 < points_.size(); i++) {
        const Vec2 a =
            remap(points_[i], frame.data_window(), frame.scene_area());
        const Vec2 b =
            remap(points_[i + 1], frame.data_window(), frame.scene_area());
        const float length = (a - b).length();
        if (frame.scene_area().intersects_segment(a, b) && length > 0) {
          const Vec2 dir = (b - a) / length;
          const float s0 =
              phase < dashed_segment_length ? -phase : period - phase;
          for (float s = s0; s < length; s += period) {
            scene.draw_line(
                a + std::max(s, 0.f) * dir,
                a + std::min(s + dashed_segment_length, length) * dir,
                args_.line_width,
                args_.color,
                false);
          }
        }
        phase = std::fmod(phase + length, period);
      }
      break;
    }
    case LineStyle::None:
      // Do nothing
      break;
  }
  for (const auto& point : points_) {
    plot_marker(point);
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
