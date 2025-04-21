#pragma once

#include "datagui/input/event.hpp"
#include "datagui/tree/tree.hpp"
#include <unordered_map>

namespace datagui {

class ElementSystems;

class ElementSystem {
public:
  virtual int type() const;
  virtual void register_type(Tree& tree, ElementSystems& systems) = 0;

  virtual void set_layout_input(Element element) const = 0;
  virtual void set_child_layout_output(Element element) const {}
  virtual void render(ConstElement element) const = 0;

  // Mouse press, hold or release inside the element bounding box
  virtual void mouse_event(Element element, const MouseEvent& event) {}
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

  void set_layout_input(Element element) const {
    systems.at(element.type())->set_layout_input(element);
  }
  void set_child_layout_output(Element element) const {
    systems.at(element.type())->set_child_layout_output(element);
  }
  void render(ConstElement element) const {
    systems.at(element.type())->render(element);
  }

  void mouse_event(Element element, const MouseEvent& event) {
    systems.at(element.type())->mouse_event(element, event);
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

} // namespace datagui
