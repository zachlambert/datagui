#pragma once

#include "datagui/geometry.hpp"

namespace datagui {

enum class MouseButton { Left, Middle, Right };
static constexpr std::size_t MouseButtonSize = 3;

enum class MouseAction { Press, Release, Hold };

struct MouseEvent {
  MouseButton button;
  MouseAction action;
  Vecf position;
};

enum class Key {
  Left,
  Right,
  Up,
  Down,
  Tab,
  Escape,
  Enter,
  Backspace,
};

enum class KeyAction { Press, Repeat, Release };

struct KeyEvent {
  Key key;
  KeyAction action;
  bool mod_ctrl;
  bool mod_shift;
};

struct TextEvent {
  char value;
};

} // namespace datagui
