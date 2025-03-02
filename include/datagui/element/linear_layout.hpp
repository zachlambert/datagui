#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"

namespace datagui {

enum class LayoutDirection { Horizontal, Vertical };

struct LinearLayout {
  float length;
  float width;
  LayoutDirection direction;

  LinearLayout(float length, float width, LayoutDirection direction) :
      length(length), width(width), direction(direction) {}

  class Handle {
  public:
    Handle& horizontal() {
      data->direction = LayoutDirection::Horizontal;
      return *this;
    }
    Handle& vertical() {
      data->direction = LayoutDirection::Vertical;
      return *this;
    }

    Handle& length_fixed(float length) {
      data->length = length;
      return *this;
    }
    Handle& length_expand(float weight) {
      data->length = -weight;
      return *this;
    }
    Handle& length_tight() {
      data->length = 0;
      return *this;
    }

    Handle& width_fixed(float width) {
      data->width = width;
      return *this;
    }
    Handle& width_expand(float weight) {
      data->width = -weight;
      return *this;
    }
    Handle& width_tight() {
      data->width = 0;
      return *this;
    }

    Handle& retain_all() {
      node->retain_all = true;
      return *this;
    }

    operator bool() const {
      return changed;
    }

  private:
    Handle(LinearLayout* data, Node* node, bool changed) :
        data(data), node(node), changed(changed) {}
    LinearLayout* const data;
    Node* const node;
    const bool changed;
    friend class LinearLayoutSystem;
  };
};

class LinearLayoutSystem : public ElementSystem {
public:
  LinearLayoutSystem(const Style& style) : style(style) {}

  int create() {
    return elements.emplace(0, 0, LayoutDirection::Horizontal);
  }

  void pop(int index) override {
    elements.pop(index);
  }

  LinearLayout::Handle handle(int index, Node* node, bool changed) {
    return LinearLayout::Handle(&elements[index], node, changed);
  }

  void calculate_size_components(Node& node, const Tree& tree) const override;

  void calculate_child_dimensions(const Node& node, Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers) const override;

private:
  const Style& style;
  VectorMap<LinearLayout> elements;
};

} // namespace datagui
