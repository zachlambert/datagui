#pragma once

#include "datagui/geometry.hpp"
#include "datagui/input/event.hpp"
#include "datagui/types/unique_any.hpp"
#include "datagui/visual/renderer.hpp"
#include <vector>

namespace datagui {

class Element;
using ElementList = std::vector<Element*>;
using ConstElementList = std::vector<const Element*>;

struct Element {
  // Layout input
  Vecf fixed_size;
  Vecf dynamic_size;
  bool hidden = false;
  bool zero_size() const {
    return fixed_size == Vecf::Zero() && dynamic_size == Vecf::Zero();
  }

  // Layout output
  Vecf position;
  Vecf size;
  Boxf box() const {
    return Boxf(position, position + size);
  }

  Boxf layer_box;
  int layer = 0;
  bool floating = false;
  Boxf float_box;
  int float_priority = 0;

  // Event handling
  Boxf hitbox; // = box or float_box
  Boxf hitbox_bounds;
  bool in_focus_tree = false;
  bool focused = false;
  bool hovered = false;

  UniqueAny props;
  int system = -1;
};

class ElementSystem {
public:
  virtual void set_input_state(
      Element& element,
      const ConstElementList& children) = 0;
  virtual void set_dependent_state(
      const Element& element,
      const ElementList& children) {}

  virtual void render(const Element& element, Renderer& renderer) const = 0;

  // Mouse press, hold or release inside the element bounding box
  virtual bool mouse_event(const Element& element, const MouseEvent& event) {
    return false;
  }
  virtual bool mouse_hover(const Element& element, const Vecf& mouse_pos) {
    return false;
  }
  // If an element captures the scroll event, process it and return true
  virtual bool scroll_event(Element& element, const ScrollEvent& event) {
    return false;
  }
  // Key press, hold or release, while a node is focused
  virtual bool key_event(Element& element, const KeyEvent& event) {
    return false;
  }
  // Text input while a node is focused
  virtual bool text_event(Element& element, const TextEvent& event) {
    return false;
  }

  // Node is focused via tab instead of clicking on it
  virtual void focus_enter(Element& element) {}
  // Node is unfocused via tab, escape or clicking on another node
  // success = should the changes be retained?
  virtual bool focus_leave(Element& element, bool success) {
    return false;
  }
};

} // namespace datagui
