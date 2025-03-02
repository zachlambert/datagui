#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"
#include <string>

namespace datagui {

struct SelectionGroup {
  std::vector<std::string> choices;
  int choice;
  float max_width;
  int element_count;

  SelectionGroup(const std::vector<std::string>& choices, int default_choice, float max_width) :
      choices(choices), choice(default_choice), max_width(max_width), element_count(0) {}
};

struct SelectionElement {
  int group;
  int choice;
  SelectionElement(bool group, int choice) : group(group), choice(choice) {}
};

struct Selection {
  std::vector<std::string> choices;
  int choice;
  float max_width;

  Selection(const std::vector<std::string>& choices, int default_choice, float max_width) :
      choices(choices), choice(default_choice), max_width(max_width) {}
};

class SelectionSystem : public ElementSystem {
public:
  SelectionSystem(const Style& style, const FontStructure& font) : style(style), font(font) {}

  int create_root(const std::vector<std::string>& choices, int default_choice, float max_width) {
    if (max_width >= 0) {
      for (const auto& choice : choices) {
        float width = text_size(font, choice, 0).x;
        max_width = std::max(max_width, width);
      }
    }

    int group = groups.size();
    groups.emplace(choices, default_choice, max_width);

    groups[group].element_count++;
    return elements.emplace(group, -1);
  }

  int create_option(const Node& parent, int choice) {
    int group = elements[parent.element_index].group;
    groups[group].element_count++;
    return elements.emplace(group, choice);
  }

  void pop(int index) override {
    int group = elements[index].group;
    groups[group].element_count--;
    if (groups[group].element_count == 0) {
      groups.pop(group);
    }
    elements.pop(index);
  }

  const int* choice(const Node& node) const {
    return &groups[elements[node.element_index].group].choice;
  }

  void calculate_size_components(Node& node, const Tree& tree) const override;
  void calculate_child_dimensions(const Node& node, Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers) const override;

  bool release(const Node& node, const Vecf& mouse_pos) override;
  bool key_event(const Node& node, const KeyEvent& event) override;

  bool focus_enter(const Node& node) override {
    if (elements[node.element_index].choice == -1) {
      return true;
    }
    return false;
  }

  bool focus_leave(const Tree& tree, const Node& node, bool success, int new_focus) override {
    const auto& new_node = tree[new_focus];
    if (new_node.element != Element::Selection) {
      return true;
    }
    if (elements[new_node.element_index].group != elements[node.element_index].group) {
      return true;
    }
    return false;
  }

private:
  const Style& style;
  const FontStructure& font;
  VectorMap<SelectionGroup> groups;
  VectorMap<SelectionElement> elements;
};

} // namespace datagui
