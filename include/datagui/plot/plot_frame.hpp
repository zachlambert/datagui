#pragma once

#include "datagui/geometry/vec.hpp"
#include "datagui/render/font_registry.hpp"
#include "datagui/render/state/draw_list.hpp"
#include "datagui/theme.hpp"
#include <optional>

namespace dgui {

class PlotFrame {
  struct Args {
    float outer_padding = 10;
    float header_margin_bot = 5;
    float aside_margin_left = 5;
    float title_legend_gap = 10;
    float tick_length = 5;
    float gradient_map_width = 10;
    float legend_icon_width = 20;
    float legend_item_gap = 15;
    float legend_padding = 4;
  };

public:
  PlotFrame();

  void init(
      const std::shared_ptr<Theme>& theme,
      const std::shared_ptr<FontRegistry>& font_registry);

  void clear();

  void set_title(const std::string& title);
  void set_xlabel(const std::string& xlabel);
  void set_ylabel(const std::string& ylabel);
  void set_xlimit(float min, float max);
  void set_ylimit(float min, float max);
  void set_undistorted(bool undistorted);

  Args& args() {
    return args_;
  }

  void add_data(const Vec2& min, const Vec2& max);
  int add_legend_item(const std::string& label);
  int add_gradient_map(float min, float max, const std::string& label = "");

  void calculate(const Box2& viewport, const Box2& subview);
  void draw_frame(const Box2& viewport, DrawList& dl) const;

  const Box2& data_window() const {
    return data_window_;
  }
  const Box2& scene_area() const {
    return scene_area_;
  }
  const Box2& plot_area() const {
    return plot_area_;
  }

  const Box2& legend_icon_box(int index) const {
    if (index < 0 || index >= (int)legend_items_.size()) {
      throw std::invalid_argument("Invalid index");
    }
    return legend_items_[index].icon_box;
  }
  const Box2& gradient_map_box(size_t index) const {
    if (index < 0 || index >= (int)gradient_maps_.size()) {
      throw std::invalid_argument("Invalid index");
    }
    return gradient_maps_[index].gm_box;
  }

private:
  enum class TicksLoc { Left, Bottom, Right };
  struct Ticks {
    TicksLoc loc;
    float length = 0;
    float min_value = 0;
    float max_value = 0;
    std::string label;
    Vec2 origin;
  };

  static constexpr float ticks_number_width_em = 4;
  float ticks_depth(const Ticks& ticks) const;
  void draw_ticks(DrawList& dl, const Ticks& ticks) const;

  std::shared_ptr<Theme> theme_;
  std::shared_ptr<FontRegistry> font_registry_;

  Args args_;

  std::string title_;
  std::optional<std::pair<float, float>> xlimit_;
  std::optional<std::pair<float, float>> ylimit_;
  bool undistorted_ = false;
  Ticks xticks_;
  Ticks yticks_;

  struct LegendItem {
    std::string label;
    // Set in calculate()
    Vec2 origin;
    Box2 icon_box;
    Vec2 text_origin;
    float text_width;
    LegendItem(const std::string& label) : label(label) {}
  };
  struct GradientMap {
    // Set in calculate(), excluding input elements of ticks
    Box2 gm_box; // Box for the rendered gradient map only
    Ticks ticks;
    GradientMap(float min, float max, const std::string& label) {
      ticks.min_value = min;
      ticks.max_value = max;
      ticks.loc = TicksLoc::Right;
      ticks.label = label;
    }
  };

  std::optional<Box2> data_range_;
  std::vector<LegendItem> legend_items_;
  std::vector<GradientMap> gradient_maps_;

  // Set in calculate()

  float header_height_ = 0; // Top header for title and legend
  float aside_width_ = 0;   // Right aside box for gradient maps
  float title_width_ = 0;
  std::optional<Box2> legend_box_;

  Box2 data_window_;
  Box2 plot_area_;
  Box2 scene_area_;
};

} // namespace dgui
