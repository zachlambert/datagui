#pragma once

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

#if 0
virtual void calculate_size_components(Node& node, const Tree& tree) const = 0;
virtual void calculate_child_dimensions(const Node& node, Tree& tree) const {}

virtual void render(const Node& node, const NodeState& state, Renderers& renderers) const = 0;

virtual bool press(const Node& node, const Vecf& mouse_pos) { return false; }

virtual bool release(const Node& node, const Vecf& mouse_pos) { return false; }

virtual bool held(const Node& node, const Vecf& mouse_pos) { return false; }

virtual bool focus_enter(const Node& node) { return false; };
virtual bool focus_leave(const Tree& tree, const Node& node, bool success, int new_focus) {
  return false;
};
virtual bool key_event(const Node& node, const KeyEvent& event) { return false; };

#endif
