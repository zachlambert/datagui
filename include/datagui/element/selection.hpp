#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"
#include <string>

namespace datagui {

struct Selection {
  std::vector<std::string> choices;
  int choice;
  float max_width;

  Selection(const std::vector<std::string> &choices, int default_choice,
            float max_width)
      : choices(choices), choice(default_choice), max_width(max_width) {}
};

class SelectionSystem : public ElementSystem {
public:
  SelectionSystem(const Style &style, const FontStructure &font)
      : style(style), font(font) {}

  int create(const std::vector<std::string> &choices, int default_choice,
             float max_width) {
    if (max_width >= 0) {
      for (const auto &choice : choices) {
        float width = text_size(font, choice, 0).x;
        max_width = std::max(max_width, width);
      }
    }
    return elements.emplace(choices, default_choice, max_width);
  }

  void pop(int index) override { elements.pop(index); }

  const int *value(const Node &node) {
    return &elements[node.element_index].choice;
  }

  void calculate_size_components(Node &node, const Tree &tree) const override;

  void render(const Node &node, const NodeState &state,
              Renderers &renderers) const override;

  bool release(const Node &node, const Vecf &mouse_pos) override;

private:
  const Style &style;
  const FontStructure &font;
  VectorMap<Selection> elements;
};

} // namespace datagui
