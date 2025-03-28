#pragma once

#include "datagui/internal/data.hpp"
#include "datagui/internal/element.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/text_selection.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"

namespace datagui {

struct TextInput {
  float max_width;
  std::string initial_text;
  Data<std::string> text;

  TextInput(float max_width, const std::string& default_text) :
      max_width(max_width), initial_text(default_text), text(default_text) {}
};

class TextInputSystem : public ElementSystem {
public:
  TextInputSystem(const Style& style, const FontStructure& font) : style(style), font(font) {}

  int create(float max_width, const std::string& default_text) {
    return elements.emplace(max_width, default_text);
  }

  void pop(int index) override {
    elements.pop(index);
  }

  const std::string* value(const Node& node) {
    auto& element = elements[node.element_index];
    return element.text.value.get();
  }

  DataPtr<std::string> data_ptr(Tree& tree, const Node& node) {
    auto& element = elements[node.element_index];
    return element.text.to_ptr(&tree);
  }

  void calculate_size_components(Node& node, const Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers) const override;

  bool press(const Node& node, const Vecf& mouse_pos) override;
  bool held(const Node& node, const Vecf& mouse_pos) override;
  bool focus_enter(const Node& node) override;
  bool focus_leave(const Tree& tree, const Node& node, bool success, int new_focus) override;
  bool key_event(const Node& node, const KeyEvent& event) override;

private:
  const Style& style;
  const FontStructure& font;
  TextSelection text_selection;
  VectorMap<TextInput> elements;
};

} // namespace datagui
