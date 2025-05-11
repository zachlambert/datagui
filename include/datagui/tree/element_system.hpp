#pragma once

#include "datagui/input/event.hpp"
#include "datagui/tree/tree.hpp"
#include <stdexcept>
#include <unordered_map>

namespace datagui {

class ElementSystems;

class ElementSystem {
public:
  virtual int type() const = 0;
  virtual void register_type(Tree& tree, ElementSystems& systems) = 0;

  virtual void set_input_state(Element element) const = 0;
  virtual void set_dependent_state(Element element) const {}

  virtual void render(ConstElement element) const = 0;

  // Mouse press, hold or release inside the element bounding box
  virtual void mouse_event(Element element, const MouseEvent& event) {}
  virtual void mouse_hover(Element element, const Vecf& mouse_pos) {}
  // If an element captures the scroll event, process it and return true
  virtual bool scroll_event(Element element, const ScrollEvent& event) {
    return false;
  }
  // Key press, hold or release, while a node is focused
  virtual void key_event(Element element, const KeyEvent& event) {}
  // Text input while a node is focused
  virtual void text_event(Element element, const TextEvent& event) {}

  // Node is focused via tab instead of clicking on it
  virtual void focus_enter(Element element) {}
  // Node is unfocused via tab, escape or clicking on another node
  // success = should the changes be retained?
  virtual void focus_leave(
      Element element,
      bool success,
      ConstElement new_element) {}
};

class ElementSystems {
public:
  void register_type(int type, ElementSystem* system) {
    assert(!systems.contains(type));
    systems.emplace(type, system);
  }

  void set_input_state(Element element) const {
    systems.at(element.type())->set_input_state(element);
  }
  void set_dependent_state(Element element) const {
    systems.at(element.type())->set_dependent_state(element);
  }
  void render(ConstElement element) const {
    systems.at(element.type())->render(element);
  }

  void mouse_event(Element element, const MouseEvent& event) {
    systems.at(element.type())->mouse_event(element, event);
  }
  void mouse_hover(Element element, const Vecf& mouse_pos) {
    systems.at(element.type())->mouse_hover(element, mouse_pos);
  }
  bool scroll_event(Element element, const ScrollEvent& scroll_event) {
    return systems.at(element.type())->scroll_event(element, scroll_event);
  }

  void key_event(Element element, const KeyEvent& event) {
    systems.at(element.type())->key_event(element, event);
  }
  void text_event(Element element, const TextEvent& event) {
    systems.at(element.type())->text_event(element, event);
  }

  void focus_enter(Element element) {
    systems.at(element.type())->focus_enter(element);
  }
  void focus_leave(Element element, bool success, ConstElement new_element) {
    systems.at(element.type())->focus_leave(element, success, new_element);
  }

private:
  std::unordered_map<int, ElementSystem*> systems;
};

template <typename Data>
class ElementSystemImpl : public ElementSystem {
public:
  int type() const override final {
    return type_;
  }
  void register_type(Tree& tree, ElementSystems& systems) override final {
    type_ = tree.create_element_type<Data>();
    systems.register_type(type_, this);
  }

private:
  int type_ = -1;
};

class ElementError : public std::runtime_error {
public:
  ElementError(const std::string& message) : std::runtime_error(message) {}
};

} // namespace datagui
