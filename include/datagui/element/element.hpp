#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/input/number_input.hpp"
#include "datagui/layout.hpp"
#include "datagui/viewport/viewport.hpp"
#include "datagui/visual/image_shader.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace dgui {

enum class Type {
  Button,
  Checkbox,
  Collapsable,
  ColorPicker,
  Dropdown,
  Group,
  Popup,
  Select,
  Slider,
  Split,
  Tabs,
  TextBox,
  TextInput,
  ViewportPtr,
};
static constexpr std::size_t TypeCount = 14;

struct Button {
  // Definition
  std::string text;

  // Args
  std::optional<Color> text_color;
  int text_size = 0;

  // State
  bool down = false;
  bool released = false;
};

struct Checkbox {
  // Dependent
  float checkbox_size = 0;

  // State
  bool checked = false;
  bool changed = false;
};

struct Collapsable {
  // Args
  Layout layout;
  Length width = LengthWrap();
  Length height = LengthWrap();
  std::string label;
  std::optional<Color> header_color;
  std::optional<Color> bg_color;
  bool border = false;

  // Dependent
  Vec2 header_size;
  Box2 content_box;

  // State
  bool open = false;
  LayoutState layout_state;
};

struct ColorPicker {
  // Args
  bool always = false;

  // Dependent
  Box2 hue_wheel_box;
  Box2 lightness_box;
  mutable Image hue_wheel_image;
  mutable Image lightness_image;

  // State
  Color value;
  bool open = false;
  bool modified = false; // Modified while holding down
  bool changed = false; // Released
  bool wheel_held = false;
  bool scale_held = false;
};

struct Dropdown {
  // Definition
  std::string label;
  Direction direction;
  Layout layout;

  // Args
  LayoutState layout_state;
  bool retain = false; // Retain state after closing

  // State
  bool open = false;
};

struct Group {
  // Args
  Layout layout;
  Length width = LengthWrap();
  Length height = LengthWrap();
  std::optional<Color> bg_color;
  bool border = false;

  // Dependent
  Box2 content_box;

  // State
  LayoutState layout_state;
};

struct Popup {
  // Definition
  std::string title;
  Vec2 popup_size;

  // Args
  Layout layout;
  std::optional<Color> header_color;
  std::optional<Color> bg_color;
  bool retain = false; // Retain state after closing

  // Dependent
  float header_height;
  Box2 header_box;
  float header_text_width;
  Box2 close_button_box;
  Box2 content_box;

  // State
  bool open = false;
  bool close_button_released = false;
  LayoutState layout_state;
};

struct Select {
  // Definition
  std::vector<std::string> choices;

  // Dependent
  std::vector<Box2> choice_boxes;

  // State
  int choice = -1;
  int choice_hovered = -1;
  bool open = false;
  bool changed = false;
};

struct Slider {
  // Args
  std::optional<float> length;
  NumberType type;
  double lower;
  double upper;
  bool always = false;
  double initial_value = 0;

  // State
  double value;
  bool held = false;
  bool changed = false;
};

struct Split {
  // Definition
  Direction direction = Direction::Vertical;

  // Args
  bool fixed = true;
  Length width;
  Length height;

  // Dependent
  Box2 divider_box;

  // State
  float ratio = 0.5;
  bool held = false;
};

struct Tabs {
  // Definition
  std::vector<std::string> labels;

  // Dependent
  float header_height;
  std::vector<Box2> label_boxes;

  // State
  size_t tab = 0;
};

struct TextInput {
  // Definition
  bool editable = true;
  std::optional<NumberType> number_type;

  // Args
  std::optional<Length> width;

  // State
  std::string text;
  bool changed = false;
};

struct TextBox {
  // Definition
  std::string text;

  // Args
  std::optional<Color> text_color;
  int text_size = 0;
};

struct ViewportPtr {
  float width;
  float height;
  std::unique_ptr<Viewport> viewport;
};

} // namespace dgui
