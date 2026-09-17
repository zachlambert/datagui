#include "datagui/viewport/plotter.hpp"

namespace dgui {

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
}

void Plotter::end() {
  for (const auto& plot : plots) {
    plot->add_frame_components(frame_);
  }
}

void Plotter::draw(const Box2& viewport, DrawList& dl) {
  // Kept so that mouse and scroll events, which arrive normalized, can be
  // mapped back into gui coordinates
  viewport_ = viewport;

  frame_.calculate(viewport, subview_);
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

Vec2 Plotter::event_to_gui(const Vec2& position) const {
  // NOTE: Events are normalized against the element box, whereas the viewport
  // passed to draw() is the content box. These differ if the viewport has a
  // border, in which case the mapping is off by the border width
  return Vec2(
      viewport_.lower.x + position.x * viewport_.size_x(),
      viewport_.upper.y - position.y * viewport_.size_y());
}

Vec2 Plotter::gui_to_scene(const Vec2& position) const {
  const Box2& plot_area = frame_.plot_area();
  return Vec2(position.x - plot_area.lower.x, plot_area.upper.y - position.y);
}

Vec2 Plotter::scene_to_data(const Vec2& position) const {
  return remap(position, frame_.scene_area(), frame_.data_window());
}

void Plotter::mouse_event(const MouseEvent& event) {
  if (event.button != MouseButton::Left) {
    return;
  }

  if (event.action == MouseAction::Press) {
    if (!frame_.plot_area().contains(event_to_gui(event.position))) {
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
    pan_press_scene_ = gui_to_scene(event_to_gui(event.position));
    pan_press_window_ = frame_.data_window();
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

  // Move the window opposite to the drag, so the data follows the cursor
  const Vec2 scene = gui_to_scene(event_to_gui(event.position));
  const Vec2 delta = (scene - pan_press_scene_) * pan_press_window_.size() /
                     frame_.scene_area().size();
  subview_ =
      Box2(pan_press_window_.lower - delta, pan_press_window_.upper - delta);
}

bool Plotter::scroll_event(const ScrollEvent& event) {
  #if 0
  const Vec2 gui = event_to_gui(event.position);
  if (!frame_.plot_area().contains(gui)) {
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

  // Zoom about the cursor, so the data under it stays put
  const Box2& window = frame_.data_window();
  const Vec2 anchor = scene_to_data(gui_to_scene(gui));
  subview_ = Box2(
      anchor - (anchor - window.lower) * size_ratio,
      anchor + (window.upper - anchor) * size_ratio);

  #endif
  return true;
}

} // namespace dgui
