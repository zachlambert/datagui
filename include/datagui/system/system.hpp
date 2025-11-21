#pragma once

#include "datagui/element/tree.hpp"
#include "datagui/geometry.hpp"
#include "datagui/input/event.hpp"
#include "datagui/visual/renderer.hpp"

namespace datagui {

class System {
public:
  virtual void set_input_state(ElementPtr element) = 0;
  virtual void set_dependent_state(ElementPtr element) = 0;

  virtual void render(ConstElementPtr element, Renderer& renderer) = 0;

  // Mouse press, hold or release inside the element bounding box
  virtual bool mouse_event(ElementPtr element, const MouseEvent& event) {
    return false;
  }
  virtual bool mouse_hover(ElementPtr element, const Vecf& mouse_pos) {
    return false;
  }
  // If an element captures the scroll event, process it and return true
  virtual bool scroll_event(ElementPtr element, const ScrollEvent& event) {
    return false;
  }
  // Key press, hold or release, while a node is focused
  virtual bool key_event(ElementPtr element, const KeyEvent& event) {
    return false;
  }
  // Text input while a node is focused
  virtual bool text_event(ElementPtr element, const TextEvent& event) {
    return false;
  }

  // Node is focused via tab instead of clicking on it
  virtual void focus_enter(ElementPtr element) {}
  // Node is unfocused via tab, escape or clicking on another node
  // success = should the changes be retained?
  virtual bool focus_leave(ElementPtr element, bool success) {
    return false;
  }
};

} // namespace datagui
