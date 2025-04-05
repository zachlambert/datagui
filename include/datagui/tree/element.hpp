#pragma once

#include "datagui/input/event.hpp"
#include "datagui/tree/tree.hpp"
#include "datagui/tree/vector_map.hpp"
#include <type_traits>

namespace datagui {

class ElementSystem {
public:
  virtual int emplace() = 0;
  virtual void pop(int index) = 0;

  virtual void set_layout_input(Tree::Ptr node) const = 0;
  virtual void set_child_layout_output(Tree::Ptr node) const {}
  virtual void render(Tree::ConstPtr node) const = 0;

  virtual void mouse_event(Tree::Ptr node, const MouseEvent& event) {}
  virtual void key_event(Tree::Ptr node, const KeyEvent& event) {}
  virtual void text_event(Tree::Ptr node, const TextEvent& event) {}

  virtual void focus_enter(Tree::Ptr node) {}
  virtual void focus_leave(
      Tree::Ptr node,
      bool success,
      Tree::ConstPtr new_node) {}
};

template <typename Element>
requires std::is_default_constructible_v<Element>
class ElementSystemBase : public ElementSystem {
public:
  int emplace() override final {
    return elements.emplace();
  }
  void pop(int index) override final {
    elements.pop(index);
  }
  Element& operator[](int index) {
    return elements[index];
  }
  const Element& operator[](int index) const {
    return elements[index];
  }

protected:
  VectorMap<Element> elements;
};

} // namespace datagui
