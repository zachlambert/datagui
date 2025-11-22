#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"
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
  TextBox,
  TextInput,
};
static constexpr std::size_t TypeCount = 9;

struct Button {
  std::string text;
  bool released = false;
  bool down = false;
  Length width = LengthWrap();
};

struct Checkbox {
  bool checked = false;
  bool changed = false;
  bool var_version = 0;
};

struct Dropdown {
  std::vector<std::string> choices;
  int choice = -1;
  int choice_hovered = -1;
  bool changed = false;
  bool var_version = 0;
  bool open = false;
  Length width = LengthWrap();
};

struct Floating {
  std::string title;
  float width;
  float height;

  bool open = false;
  bool open_changed = false;
  int open_var_version = 0;
  int content_id = 0;

  Boxf title_bar_box;
  float title_bar_text_width;
  Boxf close_button_box;
  std::optional<Color> bg_color;
};

struct Labelled {
  std::string label;
};

struct Section {
  std::string label;
  bool open = false;
};

struct Series {
  float content_length = 0;
  float overrun = 0;
  float scroll_pos = 0;

  Direction direction = Direction::Vertical;
  Alignment alignment = Alignment::Center;
  Length length = LengthWrap();
  Length width = LengthDynamic(1);
  bool no_padding = false;
  std::optional<Color> bg_color;
};

struct TextBox {
  std::string text;
};

struct TextInput {
  std::string text;
  Length width = LengthDynamic(1);

  bool changed = false;
  int var_version = 0;
};

} // namespace datagui
