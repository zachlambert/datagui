#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"

namespace datagui {

enum class LayoutDirection { Horizontal, Vertical };

struct LinearLayout {
  float length = 0;
  float width = 0;
  LayoutDirection direction = LayoutDirection::Vertical;

  class Handle {
  public:
    Handle& retain_all() {
      node->retain_all = true;
      return *this;
    }

    Handle& horizontal() {
      element->direction = LayoutDirection::Horizontal;
      return *this;
    }
    Handle& vertical() {
      element->direction = LayoutDirection::Vertical;
      return *this;
    }

    Handle& length_fixed(float length) {
      element->length = length;
      return *this;
    }
    Handle& length_expand(float weight = 1) {
      element->length = -weight;
      return *this;
    }
    Handle& length_tight() {
      element->length = 0;
      return *this;
    }

    Handle& width_fixed(float width) {
      element->width = width;
      return *this;
    }
    Handle& width_expand(float weight) {
      element->width = -weight;
      return *this;
    }
    Handle& width_tight() {
      element->width = 0;
      return *this;
    }

    operator bool() const {
      return node->changed;
    }

  private:
    Handle(Node* node, LinearLayout* element) : node(node), element(element) {}
    Node* const node;
    LinearLayout* const element;
    friend class LinearLayoutSystem;
  };
};

class LinearLayoutSystem : public ElementSystem {
public:
  LinearLayoutSystem(const Style& style) : style(style) {}

  int create() {
    return elements.emplace(0, 0, LayoutDirection::Horizontal);
  }

  LinearLayout::Handle handle(Node& node) {
    return LinearLayout::Handle(&node, &elements[node.element_index]);
  }

  void pop(int index) override {
    elements.pop(index);
  }

  void calculate_size_components(Node& node, const Tree& tree) const override;

  void calculate_child_dimensions(const Node& node, Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers) const override;

private:
  const Style& style;
  VectorMap<LinearLayout> elements;
};

} // namespace datagui
