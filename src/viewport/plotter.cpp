#include "datagui/viewport/plotter.hpp"

namespace datagui {

PlotHandle Plotter::plot(const std::vector<Vec2>& points) {
  PlotItem& item = plot_items.emplace_back();
  item.points = points;
  return PlotHandle(item.args);
}

PlotHandle Plotter::plot(std::vector<Vec2>&& points) {
  PlotItem& item = plot_items.emplace_back();
  item.points = std::move(points);
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
  return PlotHandle(item.args);
}

void Plotter::begin() {
  plot_items.clear();
  xlabel_.clear();
  ylabel_.clear();
}

void Plotter::end() {
  render_content();
  printf("Render content\n");
}

void Plotter::mouse_event(const MouseEvent& event) {
  if (event.action == MouseAction::Press) {
    mouse_down_pos = event.position;
    return;
  }
  if (event.action != MouseAction::Hold || event.button != MouseButton::Left) {
    return;
  }

  offset = event.position - mouse_down_pos;
  render_content();
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
  Box2 bounds;
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

  ShapeShader::Command shapes;
  TextShader::Command texts;

  Vec2 size = framebuffer_size();
  Box2 mask(Vec2(), size);

  float text_height = fm->text_height(theme->text_font, theme->text_size);
  float lower_padding = args.tick_length + 2 * text_height + 3 * args.padding;
  float upper_padding = args.padding;

  Box2 plot_area =
      Box2(Vec2::uniform(lower_padding), size - Vec2::uniform(upper_padding));

#if 0
  auto plot_marker = [&](const Vec2& point, const PlotArgs& args) {
    switch (args.marker_style) {
    case datagui::PlotMarkerStyle::Circle:
      shapes.queue_circle(
          point,
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
    for (std::size_t i = 0; i < item.points.size() - 1; i++) {
      const Vec2& a = item.points[i];
      const Vec2& b = item.points[i + 1];
      shapes.queue_line(a, b, item.args.line_width, item.args.color, plot_area);
      plot_marker(a, item.args);
    }
    plot_marker(item.points.back(), item.args);
  }
#endif

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

  shapes.queue_box(
      Box2(Vec2(), Vec2::uniform(20)),
      Color::Red(),
      0,
      0,
      Color::Black(),
      mask);

  auto xticks = get_ticks(bounds.lower.x, bounds.upper.x);
  auto yticks = get_ticks(bounds.lower.y, bounds.upper.y);

#if 0
  for (const auto& tick : xticks) {
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x * tick.position;
    shapes.queue_line(
        pos,
        pos + Vec2(0, -args.tick_length),
        args.line_width,
        args.tick_color,
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
  }

  if (!xlabel_.empty()) {
    Vec2 text_size = fm->text_size(
        xlabel_,
        theme->text_font,
        theme->text_size,
        LengthWrap());
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x / 2 - text_size.x / 2;
    pos.y -= text_height + 2 * args.padding;
    texts.queue_text(
        fm,
        pos,
        xlabel_,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }
#endif
#if 0
  if (!ylabel_.empty()) {
    Vec2 text_size = fm->text_size(
        ylabel_,
        theme->text_font,
        theme->text_size,
        LengthWrap());
    Vec2 pos = plot_area.lower;
    pos.x -= text_height + 2 * args.padding;
    pos.y += plot_area.size().y / 2 - text_size.x / 2;
    texts.queue_text(
        fm,
        pos,
        xlabel_,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap(),
        mask);
  }
#endif

  shape_shader.draw(shapes, framebuffer_size());
  text_shader.draw(texts, framebuffer_size());
}

std::vector<Plotter::Tick> Plotter::get_ticks(float min, float max) {
  float diff = std::max(max - min, 0.1f);
  float power = 1;
  printf("%f -> %f\n", min, max);
  printf("Original: %f\n", diff);
  while (diff > 1) {
    diff /= 10;
    power++;
    printf("Up: %f, %f\n", diff, power);
  }
  while (diff < 0.1) {
    diff *= 10;
    power--;
    printf("Down: %f, %f\n", diff, power);
  }

  float resolution;
  if (diff < 0.2) {
    resolution = 0.02;
  } else if (diff < 0.5) {
    resolution = 0.05;
  } else {
    resolution = 0.1;
  }

  std::vector<Plotter::Tick> ticks;
  float value = ceil(min / resolution) * resolution;
  while (value < max) {
    float position = (value - min) / (max - min);
    ticks.push_back({position, value});
    value += resolution;
  }
  return ticks;
}

} // namespace datagui
