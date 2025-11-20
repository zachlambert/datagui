#pragma once

#include "datagui/color.hpp"
#include "datagui/geometry.hpp"
#include "datagui/layout.hpp"
#include <optional>
#include <vector>

namespace datagui {

enum class PropsType {
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

struct ButtonProps {
  static constexpr PropsType TYPE = PropsType::Button;
  std::string text;
  bool released = false;
  bool down = false;
  Length width = LengthWrap();
};

struct CheckboxProps {
  static constexpr PropsType TYPE = PropsType::Checkbox;
  bool checked = false;
  bool changed = false;
};

struct DropdownProps {
  static constexpr PropsType TYPE = PropsType::Dropdown;
  std::vector<std::string> choices;
  int choice = -1;
  int choice_hovered = -1;
  bool changed = false;
  bool open = false;
  Length width = LengthWrap();
};

struct FloatingProps {
  static constexpr PropsType TYPE = PropsType::Floating;
  std::string title;
  float width;
  float height;

  bool open = false;
  bool open_changed = false;
  int content_id = 0;

  Boxf title_bar_box;
  float title_bar_text_width;
  Boxf close_button_box;
  std::optional<Color> bg_color;
};

struct LabelledProps {
  static constexpr PropsType TYPE = PropsType::Labelled;
  std::string label;
};

struct SectionProps {
  static constexpr PropsType TYPE = PropsType::Section;
  std::string label;
  bool open = false;
};

struct SeriesProps {
  static constexpr PropsType TYPE = PropsType::Series;

  float overrun = 0;
  float scroll_pos = 0;

  Direction direction = Direction::Vertical;
  Alignment alignment = Alignment::Center;
  Length length = LengthWrap();
  Length width = LengthDynamic(1);
  bool no_padding = false;
  std::optional<Color> bg_color;
};

struct TextBoxProps {
  static constexpr PropsType TYPE = PropsType::TextBox;
  std::string text;
};

struct TextInputProps {
  static constexpr PropsType TYPE = PropsType::TextInput;
  std::string text;
  bool changed = false;
  Length width = LengthDynamic(1);
};

} // namespace datagui
