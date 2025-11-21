#pragma once

#include "datagui/geometry.hpp"
#include "datagui/input/event.hpp"
#include "datagui/tree/element.hpp"
#include "datagui/visual/renderer.hpp"
#include <assert.h>
#include <vector>

namespace datagui {

using ElementList = std::vector<Element*>;
using ConstElementList = std::vector<const Element*>;

class ElementSystem {
public:
  virtual void set_input_state(
      Element& element,
      const ConstElementList& children) = 0;
  virtual void set_dependent_state(
      Element& element,
      const ElementList& children) {}

  virtual void render(const Element& element, Renderer& renderer) = 0;

  // Mouse press, hold or release inside the element bounding box
  virtual bool mouse_event(Element& element, const MouseEvent& event) {
    return false;
  }
  virtual bool mouse_hover(Element& element, const Vecf& mouse_pos) {
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

class ElementSystemList {
public:
  template <typename System, typename... Args>
  void emplace(Args&&... args) {
    list.push_back(std::make_unique<System>(std::forward<Args>(args)...));
  }

  ElementSystem& get(const Element& element) {
    return *list[element.system];
  }
  const ElementSystem& get(const Element& element) const {
    return *list[element.system];
  }

  template <typename System>
  requires std::is_base_of_v<ElementSystem, System>
  int find() const {
    for (int i = 0; i < list.size(); i++) {
      if (dynamic_cast<const System*>(list[i].get())) {
        return i;
      }
    }
    assert(false);
    return -1;
  }

private:
  std::vector<std::unique_ptr<ElementSystem>> list;
};

} // namespace datagui
