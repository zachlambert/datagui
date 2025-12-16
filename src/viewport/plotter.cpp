#include "datagui/viewport/plotter.hpp"
#include "datagui/visual/color_map.hpp"
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

HeatmapHandle Plotter::heatmap(
    const Vec2& lower,
    const Vec2& upper,
    const std::function<float(const Vec2&)>& function,
    std::size_t width,
    std::size_t height) {
  HeatmapItem& item = heatmap_items.emplace_back();
  item.bounds = Box2(lower, upper);
  item.function = function;
  item.width = width;
  item.height = height;
  return HeatmapHandle(item.args);
}

void Plotter::impl_init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontManager>& fm) {
  this->theme = theme;
  this->fm = fm;
  fixed_shape_shader.init();
  fixed_text_shader.init(fm);
  plot_shape_shader.init();
  plot_image_shader.init();
}

void Plotter::begin() {
  plot_items.clear();
  heatmap_items.clear();
  xlabel_.clear();
  ylabel_.clear();
  default_color_i = 0;
}

void Plotter::end() {
  redraw();
}

void Plotter::redraw() {
  Camera2d fixed_camera;
  fixed_camera.position = viewport().center();
  fixed_camera.angle = 0;
  fixed_camera.size = viewport().size();

  // TODO
  Camera2d plot_camera;
  plot_camera.position = viewport().center();
  plot_camera.angle = 0;
  plot_camera.size = viewport().size();

  queue_commands();
  bind_framebuffer();

  fixed_shape_shader.draw(viewport(), fixed_camera);
  fixed_text_shader.draw(viewport(), fixed_camera);
  plot_shape_shader.draw(viewport(), plot_camera);
  plot_image_shader.draw(viewport(), plot_camera);

  unbind_framebuffer();
  fixed_shape_shader.clear();
  fixed_text_shader.clear();
  plot_shape_shader.clear();
  plot_image_shader.clear();
}

void Plotter::queue_commands() {
  Box2 bounds;
  float text_height = fm->text_height(theme->text_font, theme->text_size);
  Vec2 size = viewport().size();

  float header_size = 0;
  float title_width = 0;
  if (!title_.empty()) {
    Vec2 title_size = fm->text_size(title_, theme->text_font, theme->text_size);
    title_width = std::min(title_size.x + theme->text_padding, size.x / 2);
    header_size = title_size.y;
    fixed_text_shader.queue_text(
        Vec2(args.outer_padding, size.y - args.outer_padding),
        0,
        title_,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthFixed(title_width));
  }
  std::size_t plot_label_count = 0;
  for (const auto& item : plot_items) {
    if (!item.args.label.empty()) {
      plot_label_count++;
    }
  }
  if (plot_label_count > 0) {
    const float item_width = 80 + theme->text_padding;
    const float item_height = text_height + theme->text_padding;

    std::size_t max_cols = std::floor(
        (size.x - 2 * args.outer_padding - title_width) / item_width);
    std::size_t cols = std::min(max_cols, plot_label_count);
    std::size_t rows = plot_label_count / cols;
    if (plot_label_count % cols != 0) {
      rows++;
    }
    header_size = std::max(header_size, rows * item_height);

    Vec2 origin =
        size - Vec2::uniform(args.outer_padding) - Vec2(item_width * cols, 0);

    std::size_t i = 0;
    std::size_t j = 0;
    for (const auto& item : plot_items) {
      if (item.args.label.empty()) {
        continue;
      }

      Vec2 pos = origin;
      pos.x += j * item_width;
      pos.y -= i * item_height;

      fixed_shape_shader.queue_rect(
          Vec2(pos.x, pos.y - text_height),
          0,
          Vec2::uniform(text_height),
          item.args.color);

      fixed_text_shader.queue_text(
          pos + Vec2(text_height + theme->text_padding, 0),
          0,
          item.args.label,
          theme->text_font,
          theme->text_size,
          theme->text_color,
          LengthFixed(item_width - text_height + theme->text_padding));

      j++;
      if (j == cols) {
        j = 0;
        i++;
      }
    }
  }

  fixed_shape_shader.queue_rect(
      Vec2(),
      0,
      viewport().size(),
      Color::Clear(),
      2,
      Color::Gray(0.5));

  float left_padding = args.tick_length + 3 * text_height +
                       2 * args.inner_padding + args.outer_padding;
  float bottom_padding = args.tick_length + 2 * text_height +
                         2 * args.inner_padding + args.outer_padding;
  float right_padding = bottom_padding;
  float top_padding = std::max(
      bottom_padding,
      header_size + args.inner_padding + args.outer_padding);

  Box2 plot_area = Box2(
      Vec2(left_padding, bottom_padding),
      size - Vec2(right_padding, top_padding));
  prev_plot_area = plot_area;

  if (plot_items.empty() && heatmap_items.empty()) {
    bounds = Box2(Vec2(), Vec2(1, 1));
  } else {
    bounds.lower.x = std::numeric_limits<float>::max();
    bounds.lower.y = std::numeric_limits<float>::max();
    bounds.upper.x = -std::numeric_limits<float>::max();
    bounds.upper.y = -std::numeric_limits<float>::max();
  }
  if (!plot_items.empty()) {
    for (const auto& item : plot_items) {
      for (const auto& point : item.points) {
        bounds.lower = minimum(point, bounds.lower);
        bounds.upper = maximum(point, bounds.upper);
      }
    }
    // To account for line width
    Vec2 extra_bounds = bounds.size() * 0.02;
    bounds.lower -= extra_bounds;
    bounds.upper += extra_bounds;
  }
  if (!heatmap_items.empty()) {
    for (const auto& item : heatmap_items) {
      bounds = bounding(item.bounds, bounds);
    }
  }

  auto to_plot_position = [&](const Vec2& point) {
    Vec2 normalized = ((point - bounds.lower) / bounds.size());
    Vec2 to_subview = (normalized - subview.lower) / subview.size();
    Vec2 to_plot_area = plot_area.lower + to_subview * plot_area.size();
    return to_plot_area;
  };

  auto plot_marker = [&](const Vec2& point, const PlotArgs& args) {
    Vec2 position = to_plot_position(point);
    switch (args.marker_style) {
    case datagui::PlotMarkerStyle::Circle:
      plot_shape_shader.queue_circle(
          position,
          args.marker_width / 2,
          args.color,
          0,
          Color::Black());
      break;
    case datagui::PlotMarkerStyle::Cross: {
      Vec2 delta_up(args.marker_width / 2, args.marker_width / 2);
      Vec2 delta_down(args.marker_width / 2, -args.marker_width / 2);
      plot_shape_shader.queue_line(
          position - delta_up,
          position + delta_up,
          args.marker_width * 0.3,
          args.color);
      plot_shape_shader.queue_line(
          position - delta_down,
          position + delta_down,
          args.marker_width * 0.3,
          args.color);
    } break;
    default:
      break;
    }
  };

  auto plot_line =
      [&](const Vec2& a, const Vec2& b, const PlotArgs& args, float length) {
        Vec2 position_a = to_plot_position(a);
        Vec2 position_b = to_plot_position(b);
        float ab_length = (position_b - position_a).length();
        Vec2 dir = (position_b - position_a) / ab_length;
        switch (args.line_style) {
        case datagui::PlotLineStyle::Solid:
          plot_shape_shader
              .queue_line(position_a, position_b, args.line_width, args.color);
          break;
        case datagui::PlotLineStyle::Dashed: {
          const float resolution = 20;
          float s1 = -std::fmod(length, resolution);
          while (s1 < ab_length) {
            float s2 = s1 + resolution;
            int i = (length + s1) / resolution;
            if (i % 2 == 0) {
              plot_shape_shader.queue_line(
                  position_a + std::max(s1, 0.f) * dir,
                  position_a + std::min(s2, ab_length) * dir,
                  args.line_width,
                  args.color,
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

  for (const auto& item : plot_items) {
    float length = 0;
    for (std::size_t i = 0; i + 1 < item.points.size(); i++) {
      const Vec2& a = item.points[i];
      const Vec2& b = item.points[i + 1];
      float line_length = plot_line(a, b, item.args, length);
      plot_marker(a, item.args);
      length += line_length;
    }
    if (!item.points.empty()) {
      plot_marker(item.points.back(), item.args);
    }
  }

  for (const auto& item : heatmap_items) {
    Vec2 plot_lower = to_plot_position(item.bounds.lower);
    Vec2 plot_upper = to_plot_position(item.bounds.upper);
    if (item.image.is_loaded()) {
      plot_image_shader.queue_masked_image(
          plot_area,
          item.image,
          plot_lower,
          plot_upper - plot_lower);
      continue;
    }

    auto to_coords = [&](std::size_t i, std::size_t j) {
      Vec2 normalized = Vec2(float(j) + 0.5, item.height - (float(i) + 0.5)) /
                        Vec2(item.width, item.height);
      return item.bounds.lower + item.bounds.size() * normalized;
    };
    float min_value = item.args.min_value ? *item.args.min_value
                                          : std::numeric_limits<float>::max();
    float max_value = item.args.max_value ? *item.args.max_value
                                          : -std::numeric_limits<float>::max();
    for (std::size_t i = 0; i < item.height; i++) {
      for (std::size_t j = 0; j < item.width; j++) {
        Vec2 coords = to_coords(i, j);
        float value = item.function(to_coords(i, j));
        if (!item.args.min_value) {
          min_value = std::min(min_value, value);
        }
        if (!item.args.max_value) {
          max_value = std::max(max_value, value);
        }
      }
    }

    struct Pixel {
      std::uint8_t r, g, b, a;
    };
    std::vector<Pixel> pixels(item.width * item.height);
    for (std::size_t i = 0; i < item.height; i++) {
      for (std::size_t j = 0; j < item.width; j++) {
        auto& pixel = pixels[i * item.width + j];
        float value = item.function(to_coords(i, j));
        float s = (value - min_value) / (max_value - min_value);
        s = std::clamp(s, 0.f, 1.f);

        switch (item.args.type) {
        case HeatmapType::Viridis: {
          Vec3 color = color_map_viridis(s);
          pixel.r = color.x * 255;
          pixel.g = color.y * 255;
          pixel.b = color.z * 255;
          break;
        }
        case HeatmapType::Linear: {
          Vec3 color =
              (1 - s) * item.args.linear_min + s * item.args.linear_max;
          pixel.r = color.x * 255;
          pixel.g = color.y * 255;
          pixel.b = color.z * 255;
          break;
        }
        }
        pixel.a = 255;
      }
    }
    item.image.load(item.width, item.height, pixels.data());
    plot_image_shader.queue_masked_image(
        plot_area,
        item.image,
        plot_lower,
        plot_upper - plot_lower);
  }

  fixed_shape_shader.queue_line(
      plot_area.lower,
      plot_area.lower_right(),
      args.line_width,
      args.tick_color);
  fixed_shape_shader.queue_line(
      plot_area.lower,
      plot_area.upper_left(),
      args.line_width,
      args.tick_color);

  Vec2 subview_lower = bounds.lower + subview.lower * bounds.size();
  Vec2 subview_upper = bounds.lower + subview.upper * bounds.size();

  auto [xticks_power, xticks] = get_ticks(subview_lower.x, subview_upper.x);
  auto [yticks_power, yticks] = get_ticks(subview_lower.y, subview_upper.y);

  for (const auto& tick : xticks) {
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x * tick.position;
    fixed_shape_shader.queue_line(
        pos,
        pos + Vec2(0, -args.tick_length),
        args.line_width,
        args.tick_color);

    Vec2 label_size =
        fm->text_size(tick.label, theme->text_font, theme->text_size);
    pos.x -= label_size.x / 2;
    pos.y -= args.tick_length + args.inner_padding;

    fixed_text_shader.queue_text(
        pos,
        0,
        tick.label,
        theme->text_font,
        theme->text_size,
        theme->text_color);
  }
  if (!xticks_power.empty()) {
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x + args.inner_padding;
    pos.y -= args.tick_length + 2 * args.inner_padding + text_height;

    fixed_text_shader.queue_text(
        pos,
        0,
        xticks_power,
        theme->text_font,
        theme->text_size,
        theme->text_color);
  }

  for (const auto& tick : yticks) {
    Vec2 pos = plot_area.lower;
    pos.y += plot_area.size().y * tick.position;
    fixed_shape_shader.queue_line(
        pos,
        pos + Vec2(-args.tick_length, 0),
        args.line_width,
        args.tick_color);

    Vec2 label_size =
        fm->text_size(tick.label, theme->text_font, theme->text_size);
    pos.x -= args.tick_length + label_size.x + args.inner_padding;
    pos.y += label_size.y / 2;

    fixed_text_shader.queue_text(
        pos,
        0,
        tick.label,
        theme->text_font,
        theme->text_size,
        theme->text_color);
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

    fixed_text_shader.queue_text(
        pos,
        0,
        xticks_power,
        theme->text_font,
        theme->text_size,
        theme->text_color);
  }

  if (!xlabel_.empty()) {
    Vec2 text_size = fm->text_size(xlabel_, theme->text_font, theme->text_size);
    Vec2 pos = plot_area.lower;
    pos.x += plot_area.size().x / 2 - text_size.x / 2;
    pos.y -= (args.tick_length + text_height + 2 * args.inner_padding);
    fixed_text_shader.queue_text(
        pos,
        0,
        xlabel_,
        theme->text_font,
        theme->text_size,
        theme->text_color);
  }
  if (!ylabel_.empty()) {
    Vec2 text_size = fm->text_size(ylabel_, theme->text_font, theme->text_size);
    Vec2 pos = plot_area.lower;
    pos.x -= (args.tick_length + 3 * text_height + 2 * args.inner_padding);
    pos.y += plot_area.size().y / 2 - text_size.x / 2;
    fixed_text_shader.queue_text(
        pos,
        M_PI / 2,
        ylabel_,
        theme->text_font,
        theme->text_size,
        theme->text_color);
  }
}

void Plotter::mouse_event(const Vec2& size, const MouseEvent& event) {
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

  Vec2 delta = mouse_down_subview.size() * (mouse_down_pos - event.position) /
               prev_plot_area.size();
  subview =
      Box2(mouse_down_subview.lower + delta, mouse_down_subview.upper + delta);

  redraw();
}

bool Plotter::scroll_event(const Vec2& element_size, const ScrollEvent& event) {
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

  redraw();
  return true;
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
