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
  Dropdown,
  Floating,
  Labelled,
  Section,
  Series,
  Slider,
  TextBox,
  TextInput,
  ViewportPtr,
};
static constexpr std::size_t TypeCount = 10;

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

struct Dropdown {
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

struct Floating {
  // Definition
  std::string title;
  float width;
  float height;
  std::function<void()> closed_callback;

  // Args
  std::optional<Color> header_color;
  std::optional<Color> bg_color;

  // Dependent
  Box2 title_bar_box;
  float title_bar_text_width;
  Box2 close_button_box;

  // State
  bool open = false;
  int open_var_version = 0;
  int content_id = 0;
};

struct Labelled {
  // Definition
  std::string label;
};

struct Section {
  // Definition
  std::string label;

  // Args
  std::optional<Color> header_color;
  std::optional<Color> bg_color;
  bool border = false;
  bool tight = false;

  // Dependent
  Vec2 header_size;

  // State
  bool open = false;
};

struct Series {
  // Args
  Direction direction = Direction::Vertical;
  Alignment alignment = Alignment::Center;
  Length length = LengthWrap();
  Length width = LengthDynamic(1);
  std::optional<Color> bg_color;
  bool border = false;
  bool tight = false;

  // Dependent
  float content_length = 0;

  // State
  float overrun = 0;
  float scroll_pos = 0;
};

struct Slider {
  // Args
  Length length = LengthDynamic();
  NumberType type;
  double lower;
  double upper;

  // State
  double value;
  float position;
};

struct TextBox {
  // Definition
  std::string text;

  // Args
  std::optional<Color> text_color;
  int text_size = 0;
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

struct ViewportPtr {
  float width;
  float height;
  std::unique_ptr<Viewport> viewport;
};

} // namespace datagui
