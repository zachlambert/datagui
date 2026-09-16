#include "datagui/plot/heatmap_plot.hpp"

#include "datagui/asset/image.hpp"
#include "datagui/render/lookup/color_map.hpp"

namespace dgui {

HeatmapPlot::Builder& HeatmapPlot::Builder::label(const std::string& label) {
  args.label = label;
  return *this;
}
HeatmapPlot::Builder& HeatmapPlot::Builder::viridis() {
  args.type = HeatmapType::Viridis;
  return *this;
}
HeatmapPlot::Builder& HeatmapPlot::Builder::linear(
    const Color& min,
    const Color& max) {
  args.type = HeatmapType::Linear;
  args.linear_min = {min.r, min.g, min.b};
  args.linear_max = {max.r, max.g, max.b};
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
    height_(height) {}

void HeatmapPlot::draw_data(
    const Box2& data_box,
    const Box2& draw_box,
    Scene2d& scene) const {
  auto get_value = [&](size_t j, size_t i) {
    const Vec2 s = Vec2(j, i) / Vec2(width_ + 1, height_ + 1);
    const Vec2 value = lower_ + s * (upper_ - lower_);
    return function_(value);
  };
  float min_value = args_.min_value.value_or(std::numeric_limits<float>::max());
  float max_value =
      args_.max_value.value_or(-std::numeric_limits<float>::max());
  if (!args_.min_value || args_.max_value) {
    for (size_t i = 0; i < height_; i++) {
      for (size_t j = 0; j < width_; j++) {
        const float value = get_value(j, i);
        if (!args_.min_value) {
          min_value = std::min(min_value, value);
        }
        if (!args_.max_value) {
          max_value = std::max(max_value, value);
        }
      }
    }
  }
  ImageData data;
  data.resize(width_, height_);
  for (size_t i = 0; i < data.height(); i++) {
    for (size_t j = 0; j < data.width(); j++) {
      const float value = get_value(j, i);
      const float s =
          std::clamp((value - min_value) / (max_value - min_value), 0.f, 1.f);
      data(j, i).set(get_color(s));
    }
  }
  Vec2 draw_lower = remap(lower_, data_box, draw_box);
  Vec2 draw_upper = remap(upper_, data_box, draw_box);
  scene.draw_image(data, draw_lower, 0, draw_upper - draw_lower);
}

Vec2 HeatmapPlot::legend_item_size() const {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);
  Vec2 size = font.text_size(args_.label);
  size.x += 2 * font.text_height();
  return size;
}

void HeatmapPlot::draw_legend_item(DrawList& dl, const Vec2& origin) const {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);
  const Vec2 text_size = font.text_size(args_.label);

  const Vec2 bar_size =
      Vec2(2 * font.text_height() - theme_->text_padding, font.text_height());
  ImageData data;
  data.resize(32, 16);
  for (size_t y = 0; y < data.height(); y++) {
    for (size_t x = 0; x < data.width(); x++) {
      const float s = static_cast<float>(x) / (data.width() + 1);
      data(x, y).set(get_color(s));
    }
  }

  Image image;
  image.load(data);
  dl.draw_image(image, origin, 0, bar_size);

  if (!args_.label.empty()) {
    dl.draw_text(
        font,
        origin + Vec2(2 * font.text_height(), 0),
        Color::Black(),
        LengthWrap(),
        args_.label);
  }
}

Vec3 HeatmapPlot::get_color(float s) const {
  s = std::clamp(s, 0.f, 1.f);
  Vec3 color;
  switch (args_.type) {
    case HeatmapType::Viridis: {
      return color_map_viridis(s);
    }
    case HeatmapType::Linear: {
      return (1 - s) * args_.linear_min + s * args_.linear_max;
    }
  }
}

} // namespace dgui
