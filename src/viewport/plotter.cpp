#include "datagui/viewport/plotter.hpp"

namespace dgui {

LinePlot::Builder Plotter::plot(const std::vector<Vec2>& points) {
  plots.push_back(std::make_unique<LinePlot>(theme, font_registry, points));
  return dynamic_cast<LinePlot*>(plots.back().get())->builder();
}

LinePlot::Builder Plotter::plot(std::vector<Vec2>&& points) {
  plots.push_back(std::make_unique<LinePlot>(theme, font_registry, std::move(points)));
  return dynamic_cast<LinePlot*>(plots.back().get())->builder();
}

LinePlot::Builder Plotter::plot(
    const float* x,
    const float* y,
    std::size_t size,
    std::size_t stride) {
  std::vector<Vec2> points(size);
  for (std::size_t i = 0; i < size; i++) {
    points[i].x = x[i * (stride / sizeof(float))];
    points[i].y = y[i * (stride / sizeof(float))];
  }
  return plot(std::move(points));
}

LinePlot::Builder Plotter::plot(
    const std::vector<float>& x,
    const std::vector<float>& y) {
  if (x.size() != y.size()) {
    throw std::invalid_argument("x and y must have the same size");
  }
  return plot(x.data(), y.data(), x.size(), sizeof(float));
}

LinePlot::Builder Plotter::plot(
    const double* x,
    const double* y,
    std::size_t size,
    std::size_t stride) {
  std::vector<Vec2> points(size);
  for (std::size_t i = 0; i < size; i++) {
    points[i].x = x[i * (stride / sizeof(double))];
    points[i].y = y[i * (stride / sizeof(double))];
  }
  return plot(std::move(points));
}

LinePlot::Builder Plotter::plot(
    const std::vector<double>& x,
    const std::vector<double>& y) {
  if (x.size() != y.size()) {
    throw std::invalid_argument("x and y must have the same size");
  }
  return plot(x.data(), y.data(), x.size(), sizeof(double));
}

LinePlot::Builder Plotter::plot_function(
    const std::function<float(float)>& f,
    float x_min,
    float x_max,
    float x_resolution) {
  std::vector<Vec2> points;
  points.reserve(std::ceil((x_max - x_min) / x_resolution));
  for (float x = x_min; x <= x_max; x += x_resolution) {
    points.push_back({x, f(x)});
  }
  return plot(std::move(points));
}

LinePlot::Builder Plotter::plot_function(
    const std::function<double(double)>& f,
    double x_min,
    double x_max,
    double x_resolution) {
  std::vector<Vec2> points;
  points.reserve(std::ceil((x_max - x_min) / x_resolution));
  for (double x = x_min; x <= x_max; x += x_resolution) {
    points.push_back({static_cast<float>(x), static_cast<float>(f(x))});
  }
  return plot(std::move(points));
}

HeatmapPlot::Builder Plotter::heatmap(
    const Vec2& lower,
    const Vec2& upper,
    const std::function<float(const Vec2&)>& function,
    std::size_t width,
    std::size_t height) {
  plots.push_back(
      std::make_unique<HeatmapPlot>(
          theme,
          font_registry,
          lower,
          upper,
          function,
          width,
          height));
  return dynamic_cast<HeatmapPlot*>(plots.back().get())->builder();
}

void Plotter::init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontRegistry>& font_registry) {
  this->theme = theme;
  this->font_registry = font_registry;
  frame_.init(theme, font_registry);
}

void Plotter::begin() {
  LinePlot::reset_default_color_i();
  frame_.clear();
  plots.clear();
}

void Plotter::end() {
  for (const auto& plot : plots) {
    plot->add_frame_components(frame_);
  }
}

void Plotter::draw(const Box2& viewport, DrawList& dl) {
  frame_.calculate(viewport);
  frame_.draw_frame(viewport, dl);
  for (const auto& plot : plots) {
    plot->draw_frame_components(frame_, dl);
  }

  // The scene is cleared to the background color before the data is drawn,
  // so that the background doesn't paint over it. Drawing a box on the draw
  // list wouldn't work, since scenes are always rendered before the
  // geometry of the group they belong to
  auto scene = std::make_shared<Scene2d>();
  scene->bg_color = Color::Gray(0.9);
  for (const auto& plot : plots) {
    plot->draw_data(frame_, *scene);
  }

  // The camera covers the scene area, which has the same size as the plot
  // area, so data mapped into it with remap() is drawn 1:1 in pixels and line
  // and marker widths carry over unscaled
  const Box2& scene_area = frame_.scene_area();
  Camera2d plot_camera;
  plot_camera.position = scene_area.center();
  plot_camera.angle = 0;
  plot_camera.size = scene_area.size();

  dl.draw_scene_2d(frame_.plot_area(), plot_camera, scene);
}

void Plotter::mouse_event(const MouseEvent& event) {
  if (event.button != MouseButton::Left) {
    return;
  }
  if (event.action == MouseAction::Press) {
    if (!frame_.plot_area().contains(event.position)) {
      mouse_down_valid = false;
      return;
    }
    if (event.is_double_click) {
      subview = Box2(Vec2(), Vec2::ones());
      mouse_down_subview = subview;
      return;
    }
    mouse_down_valid = true;
    mouse_down_pos = event.position;
    mouse_down_subview = subview;
    return;
  }
  if (!mouse_down_valid || event.action != MouseAction::Hold ||
      event.button != MouseButton::Left) {
    return;
  }

  Vec2 delta = mouse_down_subview.size() * (mouse_down_pos - event.position) /
               frame_.plot_area().size();
  subview =
      Box2(mouse_down_subview.lower + delta, mouse_down_subview.upper + delta);
}

bool Plotter::scroll_event(const ScrollEvent& event) {
  if (!frame_.plot_area().contains(event.position)) {
    return false;
  }
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
  subview = Box2(centre - size / 2, centre + size / 2);

  return true;
}

} // namespace dgui
