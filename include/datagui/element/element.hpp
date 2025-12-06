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

  // State
  bool checked = false;
  int var_version = 0;
};

struct Collapsable {
  // Args
  Layout layout;
  std::string label;
  std::optional<Color> header_color;
  std::optional<Color> bg_color;
  bool border = false;
  bool tight = false;

  // Dependent
  Vec2 header_size;
  float content_length = 0;

  // State
  bool open = false;
  LayoutState layout_state;
};

struct ColorPicker {
  // Args
  std::function<void(Color)> callback;
  bool always = false;

  // Dependent
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
  std::optional<Color> bg_color;
  bool border = false;
  bool tight = false;

  // Dependent
  float content_length = 0;

  // State
  LayoutState layout_state;
};

struct Popup {
  // Definition
  std::string title;
  std::function<void()> closed_callback;

  // Args
  Layout layout;
  float width;
  float height;
  std::optional<Color> header_color;
  std::optional<Color> bg_color;

  // Dependent
  Box2 title_bar_box;
  float title_bar_text_width;
  Box2 close_button_box;

  // State
  bool open = false;
};

struct Select {
  // Definition
  std::vector<std::string> choices;
  std::function<void(int)> callback;

  // State
  int choice = -1;
  bool var_version = 0;
  int choice_hovered = -1;
  bool open = false;
  Length width = LengthWrap();
};

struct Slider {
  // Args
  std::optional<float> length;
  NumberType type;
  double lower;
  double upper;
  std::function<void(double)> callback;
  bool always = false;

  // State
  double value;
  bool held = false;
};

struct Split {
  // Definition
  Direction direction = Direction::Vertical;

  // Args
  bool fixed = true;

  // State
  float ratio = 0.5;
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
  Length width = LengthDynamic(1);

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
