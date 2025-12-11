#include "datagui/viewport/plotter.hpp"
#include <iomanip>
#include <sstream>

namespace datagui {

// Matplotlib colors
std::vector<Color> default_plot_colors = {
    Color(0.122, 0.467, 0.706),
    Color(1.000, 0.498, 0.055),
    Color(0.173, 0.627, 0.173),
    Color(0.839, 0.153, 0.157),
    Color(0.580, 0.404, 0.741),
    Color(0.549, 0.337, 0.294),
    Color(0.890, 0.467, 0.761),
    Color(0.498, 0.498, 0.498),
    Color(0.737, 0.741, 0.133),
    Color(0.090, 0.745, 0.812),
};

PlotHandle Plotter::plot(const std::vector<Vec2>& points) {
  PlotItem& item = plot_items.emplace_back();
  item.points = points;
  item.args.color = default_plot_colors[default_color_i];
  default_color_i = (default_color_i + 1) % default_plot_colors.size();
  return PlotHandle(item.args);
}

PlotHandle Plotter::plot(std::vector<Vec2>&& points) {
  PlotItem& item = plot_items.emplace_back();
  item.points = std::move(points);
  item.args.color = default_plot_colors[default_color_i];
  default_color_i = (default_color_i + 1) % default_plot_colors.size();
  return PlotHandle(item.args);
}

PlotHandle Plotter::plot(
    const float* x,
    const float* y,
    std::size_t size,
    std::size_t stride) {
  PlotItem& item = plot_items.emplace_back();
  item.points.reserve(size);
  for (std::size_t i = 0; i < size; i++) {
    item.points[i].x = x[i * stride];
    item.points[i].y = y[i * stride];
  }
  item.args.color = default_plot_colors[default_color_i];
  default_color_i = (default_color_i + 1) % default_plot_colors.size();
  return PlotHandle(item.args);
}

PlotHandle Plotter::plot(
    const double* x,
    const double* y,
    std::size_t size,
    std::size_t stride) {
  PlotItem& item = plot_items.emplace_back();
  item.points.reserve(size);
  for (std::size_t i = 0; i < size; i++) {
    item.points[i].x = x[i * stride];
    item.points[i].y = y[i * stride];
  }
  item.args.color = default_plot_colors[default_color_i];
  default_color_i = (default_color_i + 1) % default_plot_colors.size();
  return PlotHandle(item.args);
}

void Plotter::begin() {
  plot_items.clear();
  xlabel_.clear();
  ylabel_.clear();
  default_color_i = 0;
}

void Plotter::end() {
  render_content();
}

void Plotter::mouse_event(const Box2& box, const MouseEvent& event) {
  if (event.button != MouseButton::Left) {
    return;
  }
  if (event.action == MouseAction::Press) {
    if (event.is_double_click) {
      subview = Box2(Vec2(), Vec2::ones());
      mouse_down_subview = subview;
      return;
    }
    mouse_down_pos = event.position;
    mouse_down_subview = subview;
    return;
  }
  if (event.action != MouseAction::Hold || event.button != MouseButton::Left) {
    return;
  }

  Vec2 delta = (mouse_down_pos - event.position) / box.size();
  delta = maximum(delta, -subview.lower);
  delta = minimum(delta, Vec2::ones() - subview.upper);
  subview =
      Box2(mouse_down_subview.lower + delta, mouse_down_subview.upper + delta);
  render_content();
}

bool Plotter::scroll_event(const Box2& box, const ScrollEvent& event) {
  float ratio = std::exp(event.amount / 1000.f);
  Vec2 size_ratio = Vec2::ones();
  if (!event.mod.shift) {
    size_ratio.x = ratio;
  }
  if (!event.mod.ctrl) {
    size_ratio.y = ratio;
  }
  Vec2 centre = subview.center();
  Vec2 size = subview.size() * size_ratio;
  size.x = std::min(size.x, 2 * centre.x);
  size.x = std::min(size.x, 2 * (1.f - centre.x));
  size.y = std::min(size.y, 2 * centre.y);
  size.y = std::min(size.y, 2 * (1.f - centre.y));
  subview = Box2(centre - size / 2, centre + size / 2);
  return true;
}

void Plotter::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  this->theme = theme;
  this->fm = fm;
  shape_shader.init();
  text_shader.init();
}

void Plotter::impl_render() {
  ShapeShader::Command shapes;
  TextShader::Command texts;
  Box2 bounds;

  Vec2 size = framebuffer_size();
  Box2 mask(Vec2(), size);

  shapes.queue_box(mask, Color::Clear(), 0, 2, Color::Gray(0.5), mask);

  float text_height = fm->text_height(theme->text_font, theme->text_size);
  float left_padding = args.tick_length + 3 * text_height +
                       2 * args.inner_padding + args.outer_padding;
  float bottom_padding = args.tick_length + 2 * text_height +
                         2 * args.inner_padding + args.outer_padding;
  Box2 plot_area = Box2(
      Vec2(left_padding, bottom_padding),
      size - Vec2::uniform(bottom_padding));

  if (!plot_items.empty()) {
    bounds.lower.x = std::numeric_limits<float>::max();
    bounds.lower.y = std::numeric_limits<float>::max();
    bounds.upper.x = -std::numeric_limits<float>::max();
    bounds.upper.y = -std::numeric_limits<float>::max();
    for (const auto& item : plot_items) {
      for (const auto& point : item.points) {
        bounds.lower = minimum(point, bounds.lower);
        bounds.upper = maximum(point, bounds.upper);
      }
    }
  } else {
    bounds = Box2(Vec2(), Vec2(1, 1));
  }

  auto to_plot_position = [&](const Vec2& point) {
    Vec2 normalized = ((point - bounds.lower) / bounds.size());
    Vec2 to_subview = (normalized - subview.lower) / subview.size();
    Vec2 to_plot_area = plot_area.lower + to_subview * plot_area.size();
    return to_plot_area;
  };

  auto plot_marker = [&](const Vec2& point, const PlotArgs& args) {
    switch (args.marker_style) {
    case datagui::PlotMarkerStyle::Circle:
      shapes.queue_circle(
          to_plot_position(point),
          args.marker_width / 2,
          args.color,
          0,
          Color::Black(),
          plot_area);
      break;
    default:
      break;
    }
  };

  for (const auto& item : plot_items) {
    for (std::size_t i = 0; i + 1 < item.points.size(); i++) {
      const Vec2& a = item.points[i];
      const Vec2& b = item.points[i + 1];
      shapes.queue_line(
          to_plot_position(a),
          to_plot_position(b),
          item.args.line_width,
          item.args.color,
          plot_area);
      plot_marker(a, item.args);
    }
    if (!item.points.empty()) {
      plot_marker(item.points.back(), item.args);
    }
  }

  shapes.queue_line(
      plot_area.lower,
      plot_area.lower_right(),
      args.line_width,
      args.tick_color,
      mask);
  shapes.queue_line(
      plot_area.lower,
      plot_area.upper_left(),
      args.line_width,
      args.tick_color,
      mask);

  Vec2 subview_lower = bounds.lower + subview.lower * bounds.size();
  Vec2 subview_upper = bounds.upper + subview.upper * bounds.size();

  auto [xticks_power, xticks] = get_ticks(subview_lower.x, subview_upper.x);
  auto [yticks_power, yticks] = get_ticks(subview_lower.y, subview_upper.y);

  for (const auto& tick : xticks) {
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x * tick.position;
    shapes.queue_line(
        pos,
        pos + Vec2(0, -args.tick_length),
        args.line_width,
        args.tick_color,
        mask);

    Vec2 label_size = fm->text_size(
        tick.label,
        theme->text_font,
        theme->text_size,
        LengthWrap());
    pos.x -= label_size.x / 2;
    pos.y -= args.tick_length + args.inner_padding;

    texts.queue_text(
        fm,
        pos,
        0,
        tick.label,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }
  if (!xticks_power.empty()) {
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x + args.inner_padding;
    pos.y -= args.tick_length + 2 * args.inner_padding + text_height;

    texts.queue_text(
        fm,
        pos,
        0,
        xticks_power,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }

  for (const auto& tick : yticks) {
    Vec2 pos = plot_area.lower;
    pos.y += plot_area.size().y * tick.position;
    shapes.queue_line(
        pos,
        pos + Vec2(-args.tick_length, 0),
        args.line_width,
        args.tick_color,
        mask);

    Vec2 label_size = fm->text_size(
        tick.label,
        theme->text_font,
        theme->text_size,
        LengthWrap());
    pos.x -= args.tick_length + label_size.x + args.inner_padding;
    pos.y += label_size.y / 2;

    texts.queue_text(
        fm,
        pos,
        0,
        tick.label,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }
  if (!yticks_power.empty()) {
    Vec2 label_size = fm->text_size(
        yticks_power,
        theme->text_font,
        theme->text_size,
        LengthWrap());

    Vec2 pos = plot_area.lower;
    pos.y += plot_area.size().y + args.inner_padding + text_height;
    pos.x -= args.tick_length + 2 * args.inner_padding + label_size.x;

    texts.queue_text(
        fm,
        pos,
        0,
        xticks_power,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }

  if (!xlabel_.empty()) {
    Vec2 text_size = fm->text_size(
        xlabel_,
        theme->text_font,
        theme->text_size,
        LengthWrap());
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x / 2 - text_size.x / 2;
    pos.y -= (args.tick_length + text_height + 2 * args.inner_padding);
    texts.queue_text(
        fm,
        pos,
        0,
        xlabel_,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }
  if (!ylabel_.empty()) {
    Vec2 text_size = fm->text_size(
        ylabel_,
        theme->text_font,
        theme->text_size,
        LengthWrap());
    Vec2 pos = plot_area.lower;
    pos.x -= (args.tick_length + 3 * text_height + 2 * args.inner_padding);
    pos.y += plot_area.size().y / 2 - text_size.x / 2;
    texts.queue_text(
        fm,
        pos,
        M_PI / 2,
        ylabel_,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }

  shape_shader.draw(shapes, framebuffer_size());
  text_shader.draw(texts, framebuffer_size());
}

std::tuple<std::string, std::vector<Plotter::Tick>> Plotter::get_ticks(
    float min,
    float max) {

  float power = 0;
  float resolution;
  {
    float diff = std::max(max - min, 1e-12f);
    while (diff > 10) {
      diff /= 10;
      power++;
    }
    while (diff < 1) {
      diff *= 10;
      power--;
    }
    if (diff < 2) {
      resolution = 0.2;
    } else if (diff < 5) {
      resolution = 0.5;
    } else {
      resolution = 1;
    }
    resolution *= std::pow(10, power);
  }

  float display_power = 0;
  if (power < -1 || power > 2) {
    display_power = power;
  }
  float display_value_scale = std::pow(10, display_power);

  std::vector<Plotter::Tick> ticks;
  float value = ceil(min / resolution) * resolution;
  while (value < max) {
    float position = (value - min) / (max - min);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << value / display_value_scale;
    std::string label = ss.str();

    ticks.push_back({position, label});
    value += resolution;
  }

  std::string power_label;
  if (display_power != 0) {
    std::stringstream ss;
    ss << int(display_power);
    power_label = "1e" + ss.str();
  }

  return std::make_tuple(power_label, ticks);
}

} // namespace datagui
