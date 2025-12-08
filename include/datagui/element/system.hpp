#pragma once

#include "datagui/element/tree.hpp"
#include "datagui/geometry.hpp"
#include "datagui/input/event.hpp"
#include "datagui/visual/gui_renderer.hpp"

namespace datagui {

class System {
public:
  virtual void set_input_state(ElementPtr element) = 0;
  virtual void set_dependent_state(ElementPtr element) {};

  virtual void render(ConstElementPtr element, GuiRenderer& renderer) = 0;

  // Return true/false depending on if the event is handled
  virtual void mouse_event(ElementPtr element, const MouseEvent& event) {}
  virtual void mouse_hover(ElementPtr element, const Vec2& mouse_pos) {}
  virtual bool scroll_event(ElementPtr element, const ScrollEvent& event) {
    return false;
  }
  virtual void key_event(ElementPtr element, const KeyEvent& event) {}
  virtual void text_event(ElementPtr element, const TextEvent& event) {}

  // Node is focused via tab instead of clicking on it
  virtual void focus_enter(ElementPtr element) {}
  // Node is unfocused via tab, escape or clicking on another node
  // success = should the changes be retained?
  virtual void focus_leave(ElementPtr element, bool success) {}
  virtual void focus_tree_leave(ElementPtr element) {}
};

} // namespace datagui
