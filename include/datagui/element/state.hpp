#pragma once

#include "datagui/geometry/box.hpp"
#include "datagui/geometry/vec.hpp"

namespace dgui {

enum class DisplayMode {
  Inline, Disabled, Float
};

/*
 * Stores the state common to all GUI elements.
 * The majority of this state should be reset each update and fully defined
 * by the corresponding element System and it's parent
 *
 * The exception is the state at the bottom under "Persistent state" which
 * is retained each update and managed by the Gui
 */
struct State {
  void reset_input() {
    // Set defaults
    fixed_size = Vec2();
    dynamic_size = Vec2();
    display_mode = DisplayMode::Inline;
    content_mode = DisplayMode::Inline;
  }

  // Layout input
  // Set by element in System::set_independent_state()

  Vec2 fixed_size;
  Vec2 dynamic_size;
  DisplayMode display_mode = DisplayMode::Inline;
  DisplayMode content_mode = DisplayMode::Inline;

  // Parent layout output
  // Set by the parent in System::set_dependent_state()
  // Except for Window elements which set their own output state

  Vec2 position;
  Vec2 size;
  Box2 box() const {
    return Box2(position, position + size);
  }
  bool content_overflowed;

  // Child layout output
  // Set by the element in System::set_dependent_state() (after the parent)
  Box2 content_box;

  // Input state set via args
  bool hidden = false;
  int num_cells = 1;

  // Persistent state managed by Gui
  bool focus_ancestor = false;
  bool focused = false;
  bool hovered = false;
  size_t focus_index = 0;
};

} // namespace dgui
