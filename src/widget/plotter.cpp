#include "datagui/widget/plotter.hpp"

namespace dgui {

// A float holds about 7 significant digits, so a data window any smaller than
// this fraction of the values it contains can't be resolved
static constexpr float min_window_ratio = 1e-6;

LinePlot::Builder Plotter::plot(const std::vector<Vec2>& points) {
  plots.push_back(std::make_unique<LinePlot>(theme, font_registry, points));
  return dynamic_cast<LinePlot*>(plots.back().get())->builder();
}

LinePlot::Builder Plotter::plot(std::vector<Vec2>&& points) {
  plots.push_back(
      std::make_unique<LinePlot>(theme, font_registry, std::move(points)));
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
  if (!scene) {
    scene = std::make_shared<Scene2d>();
  }
  scene->clear();
}

void Plotter::end() {
  for (const auto& plot : plots) {
    plot->add_frame_components(frame_);
  }
  frame_.calculate_sizes();
}

Vec2 Plotter::min_size() const {
  return frame_.min_size();
}

void Plotter::set_dependent_state(const Box2& box) {
  frame_.calculate_positions(box, subview_);
}

void Plotter::render(const Box2& box, DrawList& dl) const {
  frame_.draw_frame(box, dl);
  for (const auto& plot : plots) {
    plot->draw_frame_components(frame_, dl);
  }

  auto scene = std::make_shared<Scene2d>();
  scene->bg_color = Color::Gray(0.9);
  for (const auto& plot : plots) {
    plot->draw_data(frame_, *scene);
  }

  const Box2& scene_area = frame_.scene_area();
  Camera2d plot_camera;
  plot_camera.position = scene_area.center();
  plot_camera.angle = 0;
  plot_camera.size = scene_area.size();

  dl.draw_scene_2d(frame_.plot_area(), plot_camera, scene);
}

void Plotter::mouse_event(const Box2& box, const MouseEvent& event) {
  if (event.button != MouseButton::Left) {
    return;
  }

  if (event.action == MouseAction::Press) {
    if (!frame_.plot_area().contains(event.position)) {
      panning_ = false;
      return;
    }
    // Double click restores the automatic fit to the data
    if (event.is_double_click) {
      panning_ = false;
      subview_ = Box2(Vec2(), Vec2::ones());
      return;
    }
    panning_ = true;
    subview_pressed_ = subview_;
    position_pressed_ =
        remap_flip_y(event.position, frame_.plot_area(), Box2::unit_box());
    return;
  }

  if (event.action == MouseAction::Release) {
    panning_ = false;
    return;
  }

  // Keep panning once started, even if the cursor leaves the plot area
  if (!panning_ || event.action != MouseAction::Hold) {
    return;
  }
  if (frame_.scene_area().empty()) {
    return;
  }

  const Vec2 position =
      remap_flip_y(event.position, frame_.plot_area(), Box2::unit_box());
  const Vec2 delta = (position - position_pressed_) * subview_pressed_.size();
  subview_ =
      Box2(subview_pressed_.lower - delta, subview_pressed_.upper - delta);
}

bool Plotter::scroll_event(const Box2& box, const ScrollEvent& event) {
  if (!frame_.plot_area().contains(event.position)) {
    return false;
  }

  // Matches Canvas2d, where a positive scroll amount zooms out. Shift and
  // ctrl restrict the zoom to a single axis
  const float ratio = std::exp(event.amount / 250.f);
  Vec2 size_ratio = Vec2::ones();
  if (!event.mod.shift) {
    size_ratio.x = ratio;
  }
  if (!event.mod.ctrl) {
    size_ratio.y = ratio;
  }

  // Stop zooming in once the window reaches the limit of what the data's
  // values can resolve, rather than letting it collapse onto a single
  // representable value
  const Box2 window = frame_.data_window();
  for (std::size_t axis = 0; axis < 2; axis++) {
    if (size_ratio(axis) < 1 &&
        window.size()(axis) * size_ratio(axis) <
            std::abs(window.center()(axis)) * min_window_ratio) {
      size_ratio(axis) = 1;
    }
  }
  if (size_ratio.x == 1 && size_ratio.y == 1) {
    return true;
  }

  const Vec2 anchor =
      remap_flip_y(event.position, frame_.plot_area(), subview_);
  subview_ = Box2(
      anchor - (anchor - subview_.lower) * size_ratio,
      anchor + (subview_.upper - anchor) * size_ratio);

  if (panning_) {
    subview_pressed_ = subview_;
    position_pressed_ = remap_flip_y(event.position, frame_.plot_area(), Box2::unit_box());
  }

  return true;
}

} // namespace dgui
