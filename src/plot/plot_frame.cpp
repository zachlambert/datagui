#include "datagui/plot/plot_frame.hpp"
#include <iomanip>
#include <sstream>

namespace dgui {

PlotFrame::PlotFrame() {
  xticks_.loc = TicksLoc::Bottom;
  yticks_.loc = TicksLoc::Left;
}

void PlotFrame::init(
    const std::shared_ptr<Theme>& theme,
    const std::shared_ptr<FontRegistry>& font_registry) {
  this->theme_ = theme;
  this->font_registry_ = font_registry;
}

void PlotFrame::clear() {
  args_ = Args();
  title_.clear();
  xticks_.label.clear();
  yticks_.label.clear();
  xlimit_.reset();
  ylimit_.reset();
  undistorted_ = false;
  data_range_.reset();
  legend_items_.clear();
  gradient_maps_.clear();
}

void PlotFrame::add_data(const Vec2& min, const Vec2& max) {
  if (!data_range_) {
    data_range_ = Box2(min, max);
  } else {
    data_range_->lower = minimum(data_range_->lower, min);
    data_range_->upper = maximum(data_range_->upper, max);
  }
}

int PlotFrame::add_legend_item(const std::string& label) {
  legend_items_.emplace_back(label);
  return legend_items_.size() - 1;
}

int PlotFrame::add_gradient_map(
    float min,
    float max,
    const std::string& label) {
  gradient_maps_.emplace_back(min, max, label);
  return gradient_maps_.size() - 1;
}

void PlotFrame::set_title(const std::string& title) {
  title_ = title;
}

void PlotFrame::set_xlabel(const std::string& xlabel) {
  xticks_.label = xlabel;
}

void PlotFrame::set_ylabel(const std::string& ylabel) {
  yticks_.label = ylabel;
}

void PlotFrame::set_xlimit(float min, float max) {
  xlimit_ = std::make_pair(min, max);
}

void PlotFrame::set_ylimit(float min, float max) {
  ylimit_ = std::make_pair(min, max);
}

void PlotFrame::set_undistorted(bool undistorted) {
  undistorted_ = undistorted;
}

void PlotFrame::calculate_sizes() {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);

  header_height_ = 0;

  if (!title_.empty()) {
    const Vec2 title_size = font.text_size(title_);
    title_width_ = title_size.x + 2 * theme_->text_padding;
    header_height_ =
        std::max(header_height_, title_size.y + 2 * theme_->text_padding);
  } else {
    title_width_ = 0;
  }

  if (!legend_items_.empty()) {
    const float row_height = font.text_height() + 2 * theme_->text_padding;

    float x = 0;
    float y = 0;
    float content_width = 0;
    for (auto& item : legend_items_) {
      const float label_width = font.text_size(item.label).x;
      const float other_width = args_.legend_icon_width +
                                args_.legend_icon_text_padding +
                                2 * theme_->text_padding;
      const float item_width =
          std::min(label_width + other_width, args_.legend_max_width);
      if (x > 0 &&
          x + args_.legend_item_gap + item_width > args_.legend_max_width) {
        content_width = std::max(content_width, x);
        x = 0;
        y += row_height;
      }
      if (x > 0) {
        x += args_.legend_item_gap;
      }

      item.origin = Vec2(x, y);
      item.text_width = item_width - other_width;

      x += item_width;
    }
    y += row_height;
    content_width = std::max(content_width, x);

    legend_size_ =
        Vec2(content_width, y) + Vec2::uniform(2 * args_.legend_padding);
    header_height_ = std::max(header_height_, legend_size_.y);
  } else {
    legend_size_ = Vec2();
  }

  aside_width_ = 0;
  {
    for (auto& gm : gradient_maps_) {
      aside_width_ += args_.gradient_map_width + ticks_depth(gm.ticks);
    }
  }

  plot_offset_lower_.x = ticks_depth(yticks_) + args_.outer_padding;
  plot_offset_upper_.x =
      args_.outer_padding +
      (aside_width_ > 0 ? aside_width_ + args_.aside_margin_left : 0.f);
  plot_offset_lower_.y =
      header_height_ + args_.outer_padding + args_.header_margin_bot;
  plot_offset_upper_.y = ticks_depth(xticks_) + args_.outer_padding;

  min_size_ = plot_offset_lower_ + plot_offset_upper_ +
              Vec2::uniform(args_.min_plot_area_size);
  min_size_.x = std::max(
      min_size_.x,
      2 * args_.outer_padding + title_width_ + legend_size_.x +
          args_.title_legend_gap);
}

void PlotFrame::calculate_positions(const Box2& box, const Box2& subview) {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);

  if (!legend_items_.empty()) {
    legend_box_ = Box2::from_lower_right(
        box.lower_right(Vec2::uniform(args_.outer_padding)),
        legend_size_);
    const Vec2 content_origin =
        legend_box_.lower + Vec2::uniform(args_.legend_padding);
    const float row_height = font.text_height() + 2 * theme_->text_padding;

    for (auto& item : legend_items_) {
      item.origin += content_origin;
      item.icon_box = Box2::from_size(
          item.origin,
          Vec2(args_.legend_icon_width, row_height));
      item.text_origin =
          item.origin +
          Vec2(args_.legend_icon_width + args_.legend_icon_text_padding, 0) +
          Vec2::uniform(theme_->text_padding);
    }
  }

  if (!gradient_maps_.empty()) {
    const Vec2 origin = box.lower_right(
        Vec2(args_.outer_padding + aside_width_, plot_offset_lower_.y));
    const float item_height =
        box.size_y() - (plot_offset_lower_.y + plot_offset_upper_.y);

    float x = 0;
    for (auto& gm : gradient_maps_) {
      const Vec2 item_origin = origin + Vec2(x, 0);
      gm.ticks.length = item_height;
      gm.gm_box = Box2::from_lower_left(
          item_origin,
          Vec2(args_.gradient_map_width, item_height));
      // Ticks run up from the bottom-right corner of the gradient map
      gm.ticks.origin =
          item_origin + Vec2(args_.gradient_map_width, item_height);

      x += args_.gradient_map_width + ticks_depth(gm.ticks);
    }
  }

  plot_area_ =
      Box2(box.lower + plot_offset_lower_, box.upper - plot_offset_upper_);

  const Vec2 axis_origin(plot_area_.lower.x, plot_area_.upper.y);
  xticks_.origin = axis_origin;
  xticks_.length = plot_area_.size_x();
  yticks_.origin = axis_origin;
  yticks_.length = plot_area_.size_y();

  // Scene area = Selected area for the scene2d, which is a different coordinate
  // frame (y-up) chosen to have the same size as the plot area, so line widths,
  // marker sizes, text size, are scaled the same as the reset of the GUI
  scene_area_ = Box2(Vec2(), plot_area_.size());

  // =============================
  // Calculate data_fit_ and data_window_

  if (data_range_) {
    data_window_ = *data_range_;
  } else {
    // No data defined
    // If there also isn't any fixed limits defined, use an arbitrary
    // [0, 1] range
    if (!xlimit_.has_value()) {
      data_window_.lower.x = 0;
      data_window_.upper.x = 1;
    }
    if (!ylimit_.has_value()) {
      data_window_.lower.y = 0;
      data_window_.upper.y = 1;
    }
  }
  if (xlimit_.has_value()) {
    data_window_.lower.x = xlimit_->first;
    data_window_.upper.x = xlimit_->second;
  }
  if (ylimit_.has_value()) {
    data_window_.lower.y = ylimit_->first;
    data_window_.upper.y = ylimit_->second;
  }

  data_window_ = data_window_.subview(subview);

  if (undistorted_ && !plot_area_.empty()) {
    const Vec2 centre = data_window_.center();
    const Vec2 size = data_window_.size();
    const float scale = std::max(
        std::abs(size.x) / plot_area_.size_x(),
        std::abs(size.y) / plot_area_.size_y());
    const Vec2 half_size = plot_area_.size() * scale / 2;
    data_window_ = Box2(centre - half_size, centre + half_size);
  }

  xticks_.min_value = data_window_.lower.x;
  xticks_.max_value = data_window_.upper.x;
  yticks_.min_value = data_window_.lower.y;
  yticks_.max_value = data_window_.upper.y;
}

void PlotFrame::draw_frame(const Box2& viewport, DrawList& dl) const {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);

  if (!title_.empty()) {
    // draw_text takes the top-left of the text, and gui coordinates are
    // y-down, so the header is at viewport.lower
    dl.draw_text(
        font,
        viewport.lower +
            Vec2::uniform(args_.outer_padding + theme_->text_padding),
        theme_->text_color,
        LengthFixed(title_width_),
        title_);
  }

  if (!legend_items_.empty()) {
    dl.draw_box(legend_box_, Color::White(), 2);
    for (const auto& item : legend_items_) {
      dl.draw_text(
          font,
          item.text_origin,
          theme_->text_color,
          LengthFixed(item.text_width),
          item.label);
    }
  }

  for (const auto& gradient_map : gradient_maps_) {
    dl.draw_box(gradient_map.gm_box, Color::Black());
    draw_ticks(dl, gradient_map.ticks);
  }

  // NOTE: Don't draw a background color, since it will be drawn over the plot
  // data Instead, the scene2d is given a bg color
  dl.draw_line(
      plot_area_.lower_left(),
      plot_area_.upper_left(),
      2,
      Color::Black());
  dl.draw_line(
      plot_area_.upper_left(),
      plot_area_.upper_right(),
      2,
      Color::Black());

  draw_ticks(dl, xticks_);
  draw_ticks(dl, yticks_);
}

float PlotFrame::ticks_depth(const Ticks& ticks) const {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);
  float depth = args_.tick_length;
  depth += theme_->text_padding;
  depth += ticks.loc == TicksLoc::Bottom
               ? font.text_height()
               : font.text_height() * ticks_number_width_em;
  depth += theme_->text_padding;
  if (!ticks.label.empty()) {
    // Either takes one line (possibly overflowing), or user manually
    // puts a newline in the label if required to avoid overflow
    depth += font.text_size(ticks.label).y + theme_->text_padding;
  }
  return depth;
}

void PlotFrame::draw_ticks(DrawList& dl, const Ticks& ticks) const {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);

  float power = 0;
  float diff = std::max(ticks.max_value - ticks.min_value, 1e-12f);
  if (!std::isfinite(diff)) {
    // Silently ignore
    return;
  }
  while (diff > 10) {
    diff /= 10;
    power++;
  }
  while (diff < 1) {
    diff *= 10;
    power--;
  }
  float resolution;
  if (diff < 2) {
    resolution = 0.2;
  } else if (diff < 5) {
    resolution = 0.5;
  } else {
    resolution = 1;
  }
  resolution *= std::pow(10, power);

  float display_power = 0;
  if (power < -1 || power > 2) {
    display_power = power;
  }
  float display_value_scale = std::pow(10, display_power);

  float value = ceil(ticks.min_value / resolution) * resolution;
  while (value < ticks.max_value) {
    float s = (value - ticks.min_value) / (ticks.max_value - ticks.min_value);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << value / display_value_scale;
    const std::string text = ss.str();

    // Use the natural size to position the label. Measuring with a fixed
    // width would just return that width back, leaving the label detached
    // from its tick
    const Vec2 text_size = font.text_size(text);

    Vec2 position = ticks.origin;
    Vec2 end_offset;
    Vec2 text_offset;
    switch (ticks.loc) {
      case TicksLoc::Left:
        // Gui coordinates are y-down, so increasing values run up the screen
        position.y -= s * ticks.length;
        end_offset.x = -args_.tick_length;
        text_offset = Vec2(
            -args_.tick_length - theme_->text_padding - text_size.x,
            -text_size.y / 2);
        break;
      case TicksLoc::Right:
        position.y -= s * ticks.length;
        end_offset.x = args_.tick_length;
        text_offset =
            Vec2(args_.tick_length + theme_->text_padding, -text_size.y / 2);
        break;
      case TicksLoc::Bottom:
        position.x += s * ticks.length;
        end_offset.y = args_.tick_length;
        text_offset =
            Vec2(-text_size.x / 2, args_.tick_length + theme_->text_padding);
        break;
    }
    dl.draw_line(position, position + end_offset, 2, Color::Black());
    dl.draw_text(
        font,
        position + text_offset,
        theme_->text_color,
        LengthWrap(),
        text);

    value += resolution;
  }

  // Draw the display power off the end of the ticks line
  // Adjust margins so there is always enough margin for this to fit
  if (display_power != 0) {
    std::stringstream ss;
    ss << int(display_power);
    const std::string power_label = "1e" + ss.str();
    const float width = font.text_size(power_label).x;

    Vec2 text_offset;
    switch (ticks.loc) {
      case TicksLoc::Left:
        text_offset = Vec2(
            -width - theme_->text_padding,
            -ticks.length - font.text_height() - theme_->text_padding);
        break;
      case TicksLoc::Right:
        text_offset = Vec2(
            theme_->text_padding,
            -ticks.length - font.text_height() - theme_->text_padding);
        break;
      case TicksLoc::Bottom:
        text_offset = Vec2(
            ticks.length + theme_->text_padding,
            args_.tick_length + theme_->text_padding);
        break;
    }
    dl.draw_text(
        font,
        ticks.origin + text_offset,
        theme_->text_color,
        LengthWrap(),
        power_label);
  }

  if (!ticks.label.empty()) {
    // TODO
  }
}

} // namespace dgui
