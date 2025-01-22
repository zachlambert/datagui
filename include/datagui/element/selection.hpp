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
  bool open;

  Selection(const std::vector<std::string>& choices, int default_choice, float max_width) :
      choices(choices), choice(default_choice), max_width(max_width), open(false) {}
};

class SelectionSystem : public ElementSystem {
public:
  SelectionSystem(const Style& style, const FontStructure& font) : style(style), font(font) {}

  int create(const std::vector<std::string>& choices, int default_choice, float max_width) {
    if (max_width >= 0) {
      for (const auto& choice : choices) {
        float width = text_size(font, choice, 0).x;
        max_width = std::max(max_width, width);
      }
    }
    return elements.emplace(choices, default_choice, max_width);
  }

  void pop(int index) override { elements.pop(index); }

  bool open(const Node& node) const { return elements[node.element_index].open; }
  // const int* value(const Node& node) { return &elements[node.element_index].choice; }

  void calculate_size_components(Node& node, const Tree& tree) const override;
  void calculate_child_dimensions(const Node& node, Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers) const override;

  bool release(const Node& node, const Vecf& mouse_pos) override;
  bool focus_enter(const Node& node) override;
  bool focus_leave(const Tree& tree, const Node& node, bool success, int new_focus) override;

private:
  const Style& style;
  const FontStructure& font;
  VectorMap<Selection> elements;
};

struct Option {
  std::string choice;
  float max_width;
  bool is_selected;

  Option(const std::string& choice, float max_width) :
      choice(choice), max_width(max_width), is_selected(false) {}
};

class OptionSystem : public ElementSystem {
public:
  OptionSystem(const Style& style, const FontStructure& font) : style(style), font(font) {}

  int create(const std::string& choice, float max_width) {
    return elements.emplace(choice, max_width);
  }

  void pop(int index) override { elements.pop(index); }

  bool is_selected(const Node& node) { return elements[node.element_index].is_selected; }

  void calculate_size_components(Node& node, const Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers) const override;

  bool release(const Node& node, const Vecf& mouse_pos) override;
  bool focus_leave(const Tree& tree, const Node& node, bool success, int new_focus) override;

private:
  const Style& style;
  const FontStructure& font;
  VectorMap<Option> elements;
};

} // namespace datagui
