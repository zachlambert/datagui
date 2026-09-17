#include "datagui/plot/plot_frame.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace dgui {

namespace {

// The power of ten of a value's magnitude, ie floor(log10(abs(value))), and
// zero for a value of zero
double magnitude_power(double value) {
  double magnitude = std::abs(value);
  if (magnitude == 0 || !std::isfinite(magnitude)) {
    return 0;
  }
  double power = 0;
  while (magnitude >= 10) {
    magnitude /= 10;
    power++;
  }
  while (magnitude < 1) {
    magnitude *= 10;
    power--;
  }
  return power;
}

} // namespace

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

  // Min size to allow for power/offset display at top of y axis
  const float min_padding_title_plot = 1.2 * font.text_height();

  header_height_ = 0;

  if (!title_.empty()) {
    const Vec2 title_size = font.text_size(title_);
    title_width_ = title_size.x + 2 * theme_->text_padding;
    header_height_ =
        title_size.y + 2 * theme_->text_padding + min_padding_title_plot;
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
  // Set a minimum aside width to allow for the xaxis power label
  aside_width_ = std::max(aside_width_, font.text_height() * 3.2f);

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

float PlotFrame::ticks_number_depth(const Ticks& ticks) const {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);
  // Numbers below the axis only need one line of height, whereas numbers
  // beside the axis are given a fixed em width
  return ticks.loc == TicksLoc::Bottom
             ? font.text_height()
             : font.text_height() * ticks_number_width_em;
}

float PlotFrame::ticks_depth(const Ticks& ticks) const {
  const auto& font =
      font_registry_->get_font(theme_->text_font, theme_->text_size);
  float depth = args_.tick_length;
  depth += theme_->text_padding;
  depth += ticks_number_depth(ticks);
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

  // Zooming in shrinks the range far below what a float can resolve at the
  // magnitude of the values, so the values are calculated at double precision
  const double min_value = ticks.min_value;
  const double max_value = ticks.max_value;

  double power = 0;
  double diff = std::max(max_value - min_value, 0.0);
  if (!std::isfinite(diff) || diff == 0.0) {
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

  // Guaranteed that diff ~ [1, 10]
  // Choose the display_resolution from a set of "nice" numbers such
  // that diff / display_resolution < max_count

  int max_count = ticks_max_count;
  if (ticks.loc == TicksLoc::Bottom) {
    max_count = std::min<int>(
        max_count,
        std::ceil(
            plot_area_.size_x() /
            (font.text_height() * ticks_number_width_em)));
  } else {
    max_count = std::min<int>(
        max_count,
        std::ceil(plot_area_.size_y() / font.text_height()));
  }

  double resolution_display;
  std::array<double, 10> display_resolution_choices =
      {0.1, 0.25, 0.4, 0.5, 1.0, 2.5, 4.0, 5.0, 8.0, 10.0};
  for (double choice : display_resolution_choices) {
    int count = std::ceil(diff / choice);
    resolution_display = choice;
    if (count < max_count) {
      break;
    }
  }
  const double resolution = resolution_display * std::pow(10, power);

  // The power to scale by comes from the magnitude of what is displayed, not
  // from the range. Powers close to zero are left unscaled, since they are
  // readable as they are
  auto display_power_for = [](double magnitude) {
    const double value_power = magnitude_power(magnitude);
    return (value_power < -1 || value_power > 2) ? value_power : 0.0;
  };
  // Consecutive ticks are separated by mantissa * 10^(power - display_power)
  // once scaled, which is how many decimal places are needed to tell them
  // apart
  auto precision_for = [&](double display_power) {
    return int(display_power - power) + (resolution_display < 1 ? 1 : 0);
  };

  double offset = 0;
  double display_power =
      display_power_for(std::max(std::abs(min_value), std::abs(max_value)));

  if (precision_for(display_power) > ticks_max_precision) {
    // Zoomed in far enough that the values share leading digits which every
    // label would otherwise repeat. Those are pulled out into a single offset,
    // leaving the labels to show only the part that varies. Rounding towards
    // zero to one decade above the tick spacing keeps the remainder to under
    // ten ticks, so the labels stay short whatever the values are
    const double offset_step = std::pow(10, power + 1);
    offset = std::trunc(min_value / offset_step) * offset_step;
    display_power = display_power_for(
        std::max(std::abs(min_value - offset), std::abs(max_value - offset)));
  }

  const double display_value_scale = std::pow(10, display_power);
  const int precision =
      std::clamp(precision_for(display_power), 0, ticks_max_precision);

  // The resolution is derived from the range, so the tick count is bounded by
  // construction. Calculate it up front and index the values off the first
  // one: accumulating `value += resolution` doesn't advance at all once the
  // resolution drops below the precision available at that value
  const double first_value = std::ceil(min_value / resolution) * resolution;
  const double count = std::ceil((max_value - first_value) / resolution);
  int tick_count = std::isfinite(count)
                       ? int(std::clamp(count, 0.0, double(ticks_max_count)))
                       : 0;
  if (ticks.loc == TicksLoc::Bottom) {
    tick_count = std::min<int>(
        tick_count,
        std::floor(
            plot_area_.size_x() /
            (font.text_height() * ticks_number_width_em)));
  }

  for (int i = 0; i < tick_count; i++) {
    const double value = first_value + i * resolution;
    const float s = float((value - min_value) / (max_value - min_value));

    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision)
       << (value - offset) / display_value_scale;
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
  }

  // Draw the multiplier and offset off the end of the ticks line
  // Adjust margins so there is always enough margin for this to fit
  if (tick_count > 0 && (display_power != 0 || offset != 0)) {
    std::stringstream ss;
    if (display_power != 0) {
      ss << "1e" << int(display_power);
    }
    if (offset != 0) {
      if (display_power != 0) {
        ss << "\n";
      }
      // Increase the resolution of the offset as required, up to a maximum
      // value of 3 decimal places (currently assume this is fine, otherwise
      // it's difficult to find a suitable layout to fit the offset in without
      // extra padding on the rhs)
      ss << std::showpos << std::fixed
         << std::setprecision(std::clamp(int(-(power + 1)), 0, 3)) << offset;
    }
    const std::string power_label = ss.str();
    const Vec2 label_size = font.text_size(power_label);

    Vec2 text_offset;
    switch (ticks.loc) {
      case TicksLoc::Left:
        text_offset = Vec2(
            -label_size.x - theme_->text_padding,
            -ticks.length - label_size.y - theme_->text_padding);
        break;
      case TicksLoc::Right:
        text_offset = Vec2(
            theme_->text_padding,
            -ticks.length - label_size.y - theme_->text_padding);
        break;
      case TicksLoc::Bottom:
        // Small extra x padding to avoid overlap
        // Doesn't need to be particularly big, since when the power/offset label appears,
        // the numbers being shown are limited to being small as well
        text_offset = Vec2(
            ticks.length + theme_->text_padding + font.text_height() * 0.7f,
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
    // The label sits beyond the tick numbers, centered along the axis. Labels
    // beside the axis are rotated to read bottom-to-top, so their text height
    // is what extends away from the axis in both cases.
    const Vec2 label_size = font.text_size(ticks.label);
    const float label_offset = args_.tick_length + 2 * theme_->text_padding +
                               ticks_number_depth(ticks);

    Vec2 text_offset;
    float angle = 0;
    switch (ticks.loc) {
      case TicksLoc::Left:
        // Rotating by -90 degrees maps the text origin (its top-left corner)
        // to the bottom-left corner of the rendered label
        angle = -M_PIf / 2;
        text_offset = Vec2(
            -label_offset - label_size.y,
            -(ticks.length - label_size.x) / 2);
        break;
      case TicksLoc::Right:
        angle = -M_PIf / 2;
        text_offset = Vec2(label_offset, -(ticks.length - label_size.x) / 2);
        break;
      case TicksLoc::Bottom:
        text_offset = Vec2((ticks.length - label_size.x) / 2, label_offset);
        break;
    }
    dl.draw_text(
        font,
        ticks.origin + text_offset,
        theme_->text_color,
        LengthWrap(),
        ticks.label,
        false,
        angle);
  }
}

} // namespace dgui
