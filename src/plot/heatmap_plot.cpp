#include "datagui/plot/heatmap_plot.hpp"

#include "datagui/asset/image.hpp"

namespace dgui {

HeatmapPlot::Builder& HeatmapPlot::Builder::label(const std::string& label) {
  args.label = label;
  return *this;
}
HeatmapPlot::Builder& HeatmapPlot::Builder::viridis() {
  args.gradient_map = GradientMap::viridis();
  return *this;
}
HeatmapPlot::Builder& HeatmapPlot::Builder::linear(
    const Color& min,
    const Color& max) {
  args.gradient_map = GradientMap::linear(min, max);
  return *this;
}
HeatmapPlot::Builder& HeatmapPlot::Builder::min_value(float value) {
  args.min_value = value;
  return *this;
}
HeatmapPlot::Builder& HeatmapPlot::Builder::max_value(float value) {
  args.max_value = value;
  return *this;
}

HeatmapPlot::HeatmapPlot(
    std::shared_ptr<Theme> theme,
    std::shared_ptr<FontRegistry> font_registry,
    const Vec2& lower,
    const Vec2& upper,
    const std::function<float(const Vec2&)>& function,
    size_t width,
    size_t height) :
    theme_(theme),
    font_registry_(font_registry),
    lower_(lower),
    upper_(upper),
    function_(function),
    width_(width),
    height_(height) {

  auto get_value = [&](size_t j, size_t i) {
    const Vec2 s = Vec2(j, i) / Vec2(width_ - 1, height_ - 1);
    const Vec2 value = lower_ + s * (upper_ - lower_);
    return function_(value);
  };
  data_min_ = std::numeric_limits<float>::max();
  data_max_ = -std::numeric_limits<float>::max();
  for (size_t i = 0; i < height_; i++) {
    for (size_t j = 0; j < width_; j++) {
      const float value = get_value(j, i);
      if (!args_.min_value) {
        data_min_ = std::min(data_min_, value);
      }
      if (!args_.max_value) {
        data_max_ = std::max(data_max_, value);
      }
    }
  }
}

void HeatmapPlot::add_frame_components(PlotFrame& frame) {
  frame.add_data(lower_, upper_);
  icon_index_ = frame.add_gradient_map(
      args_.min_value.value_or(data_min_),
      args_.max_value.value_or(data_max_),
      args_.label);
}

void HeatmapPlot::draw_frame_components(const PlotFrame& frame, DrawList& dl)
    const {
  if (icon_index_ < 0) {
    return;
  }
  const Box2& box = frame.gradient_map_box(icon_index_);
  ImageData data;
  data.resize(8, std::ceil(8 * box.size_y() / box.size_x()));
  for (size_t y = 0; y < data.height(); y++) {
    for (size_t x = 0; x < data.width(); x++) {
      const float s = static_cast<float>(x) / (data.width() - 1);
      data(x, y).set(args_.gradient_map.lookup(s).rgb);
    }
  }

  Image image;
  image.load(data);
  dl.draw_image(image, box.lower, 0, box.size());
}

void HeatmapPlot::draw_data(const PlotFrame& frame, Scene2d& scene) const {
  auto get_value = [&](size_t j, size_t i) {
    Vec2 s = Vec2(j, i) / Vec2(width_ - 1, height_ - 1);
    s.x = std::clamp(s.x, 0.f, 1.f);
    s.y = std::clamp(s.y, 0.f, 1.f);
    const Vec2 value = lower_ + s * (upper_ - lower_);
    return function_(value);
  };
  float min_value = args_.min_value.value_or(data_min_);
  float max_value =
      args_.max_value.value_or(data_max_);
  ImageData data;
  data.resize(width_, height_);
  for (size_t i = 0; i < data.height(); i++) {
    for (size_t j = 0; j < data.width(); j++) {
      const float value = get_value(j, i);
      const float s =
          std::clamp((value - min_value) / (max_value - min_value), 0.f, 1.f);
      data(j, i).set(args_.gradient_map.lookup(s).rgb);
    }
  }
  Vec2 draw_lower = remap(lower_, frame.data_area(), frame.plot_area());
  Vec2 draw_upper = remap(upper_, frame.data_area(), frame.plot_area());
  scene.draw_image(data, draw_lower, 0, draw_upper - draw_lower);
}

} // namespace dgui
