#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/input/number_input.hpp"
#include "datagui/layout.hpp"
#include "datagui/viewport/viewport.hpp"
#include <functional>
#include <optional>
#include <vector>

namespace datagui {

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
  std::function<void()> callback;

  // Args
  std::optional<Color> text_color;
  int text_size = 0;

  // State
  bool down = false;
};

struct Checkbox {
  // Definition
  std::function<void(bool)> callback;

  // Args
  std::string label;

  // Dependent
  float checkbox_size;
  Vec2 label_size;

  // State
  bool checked = false;
  int var_version = 0;
};

struct Collapsable {
  // Args
  Layout layout;
  Length width;
  Length height;
  std::string label;
  std::optional<Color> header_color;
  std::optional<Color> bg_color;
  bool border = false;
  bool tight = false;

  // Dependent
  Vec2 header_size;
  Box2 content_box;

  // State
  bool open = false;
  LayoutState layout_state;
};

struct ColorPicker {
  // Args
  std::string label;
  std::function<void(Color)> callback;
  bool always = false;

  // Dependent
  Vec2 label_size;
  Box2 hue_wheel_box;
  Box2 lightness_box;

  // State
  Color value;
  bool open = false;
  bool modified = false;
  bool wheel_held = false;
  bool scale_held = false;
};

struct Dropdown {
  // Definition
  std::string label;
  Direction direction;
  Layout layout;

  // Args
  std::optional<Color> bg_color;

  // State
  bool open = false;
};

struct Group {
  // Args
  Layout layout;
  Length width;
  Length height;
  std::optional<Color> bg_color;
  bool border = false;
  bool tight = false;

  // Dependent
  Box2 content_box;

  // State
  LayoutState layout_state;
};

struct Popup {
  // Definition
  std::string title;
  Vec2 popup_size;
  std::function<void()> closed_callback;

  // Args
  Layout layout;
  std::optional<Color> header_color;
  std::optional<Color> bg_color;

  // Dependent
  float header_height;
  Box2 header_box;
  float header_text_width;
  Box2 close_button_box;
  Box2 content_box;

  // State
  bool open = false;
  LayoutState layout_state;
};

struct Select {
  // Definition
  std::vector<std::string> choices;
  std::function<void(int)> callback;

  // Args
  std::string label;

  // Dependent
  Vec2 label_size;
  Box2 select_box;

  // State
  int choice = -1;
  bool var_version = 0;
  int choice_hovered = -1;
  bool open = false;
};

struct Slider {
  // Args
  std::string label;
  std::optional<float> length;
  NumberType type;
  double lower;
  double upper;
  std::function<void(double)> callback;
  bool always = false;

  // Dependent
  Vec2 label_size;

  // State
  double value;
  bool held = false;
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

  // State
  std::size_t tab = 0;
};

struct TextInput {
  // Definition
  std::function<void(const std::string&)> callback;
  bool editable = true;
  std::optional<NumberType> number_type;

  // Args
  std::string label;
  std::optional<Length> width;

  // Dependent
  Vec2 label_size;
  Box2 text_input_box;

  // State
  std::string text;
  int var_version = 0;
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

} // namespace datagui
