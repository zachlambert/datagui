#pragma once

#include "datagui/geometry.hpp"

namespace datagui {

enum class MouseButton { Left, Middle, Right };
static constexpr std::size_t MouseButtonSize = 3;

enum class MouseAction { Press, Release, Hold };

struct Modifiers {
  bool ctrl = false;
  bool shift = false;
};

struct MouseEvent {
  MouseButton button;
  MouseAction action;
  Vec2 position;
  Modifiers mod;
  bool is_double_click = false;
};

struct ScrollEvent {
  Vec2 position;
  float amount;
  Modifiers mod;
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
  Delete,
  // For Ctrl-Character
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z
};

enum class KeyAction { Press, Repeat, Release };

struct KeyEvent {
  Key key;
  KeyAction action;
  Modifiers mod;
  // Only need this for handling CtrlC/CtrlV behaviour
  // Pass GLFWWindow* as a void pointer to avoid including glfw here
  void* glfw_window;
};

struct TextEvent {
  char value;
};

} // namespace datagui
