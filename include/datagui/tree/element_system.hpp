#pragma once

#include "datagui/input/event.hpp"
#include "datagui/tree/tree.hpp"
#include "datagui/tree/vector_map.hpp"
#include <type_traits>

namespace datagui {

class ElementSystem {
public:
  virtual ElementType type() const = 0;
  virtual int emplace() = 0;
  virtual void pop(int index) = 0;

  virtual void set_layout_input(Element element) const = 0;
  virtual void set_child_layout_output(Element element) const {}
  virtual void render(ConstElement element) const = 0;

  // Mouse press, hold or release inside the element bounding box
  virtual void mouse_event(Element element, const MouseEvent& event) {}
  // Key press, hold or release, while a node is focused
  virtual void key_event(Element node, const KeyEvent& event) {}
  // Text input while a node is focused
  virtual void text_event(Element element, const TextEvent& event) {}

  // Node is focused via tab instead of clicking on it
  virtual void focus_enter(Element node) {}
  // Node is unfocused via tab, escape or clicking on another node
  // success = should the changes be retained?
  virtual void focus_leave(
      Element element,
      bool success,
      ConstElement new_element) {}
};

template <ElementType Type, typename Data>
requires std::is_default_constructible_v<Data>
class ElementSystemBase : public ElementSystem {
public:
  ElementType type() const override final {
    return Type;
  }
  int emplace() override final {
    return datas.emplace();
  }
  void pop(int index) override final {
    datas.pop(index);
  }
  Data& get(int index) {
    return datas[index];
  }
  const Data& get(int index) const {
    return datas[index];
  }

protected:
  VectorMap<Element> datas;
};

} // namespace datagui
