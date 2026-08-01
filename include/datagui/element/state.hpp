#pragma once

#include "datagui/geometry.hpp"

namespace dgui {

/* Stores the state common to all GUI elements.
 * The majority of this state should be reset each update and fully defined
 * by the corresponding element System and it's parent
 *
 * The exception is the state at the bottom under "Persistent state" which
 * is retained each update and managed by the Gui
 */
struct State {
  void reset_input() {
    fixed_size = Vec2();
    dynamic_size = Vec2();
    visible = true;
    content_visible = true;
    content_floating = false;
  }
  void set_hidden() {
    visible = false;
    content_visible = false;
  }

  // Layout input
  // Set by element in System::set_independent_state()

  Vec2 fixed_size;
  Vec2 dynamic_size;
  bool visible = true;           // Is the element body visible?
  bool content_visible = false;  // Does the element have visible child content?
  bool content_floating = false; // Is the child content floating?
  bool is_popup = false;

  // Parent layout output
  // Set by the parent in System::set_dependent_state()

  Vec2 position;
  Vec2 size;
  Box2 box() const {
    return Box2(position, position + size);
  }
  // May also override visible and content_visible

  // Child layout output
  // Set by the element in System::set_dependent_state() (after the parent)
  Box2 content_box;
  bool content_overflowed = false;

  // Input state set via args
  int num_cells = 1;

  // Persistent state managed by Gui
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;
  int float_priority = 0; // Set by gui, only applicable for floating content
};

} // namespace dgui
