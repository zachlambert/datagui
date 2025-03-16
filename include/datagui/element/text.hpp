#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/text_selection.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"

namespace datagui {

struct Text {
  std::string text;
  float max_width = 0;

  class Handle {
  public:
    Handle& text(const std::string& text) {
      element->text = text;
      return *this;
    }
    Handle& max_width(float max_width) {
      element->max_width = max_width;
      return *this;
    }

  private:
    Handle(Node* const node, Text* const element) : node(node), element(element) {}
    Node* const node;
    Text* const element;
    friend class TextSystem;
  };
};

class TextSystem : public ElementSystem {
public:
  TextSystem(const Style& style, const FontStructure& font) : style(style), font(font) {}

  int create() {
    return elements.emplace();
  }

  void pop(int index) override {
    elements.pop(index);
  }

  Text::Handle handle(Node& node) {
    return Text::Handle(&node, &elements[node.element_index]);
  }

  void calculate_size_components(Node& node, const Tree& tree) const override;

  void render(const Node& node, const NodeState& state, Renderers& renderers) const override;

  bool press(const Node& node, const Vecf& mouse_pos) override;
  bool held(const Node& node, const Vecf& mouse_pos) override;
  bool focus_enter(const Node& node) override;
  bool key_event(const Node& node, const KeyEvent& event) override;

private:
  const Style& style;
  const FontStructure& font;
  TextSelection text_selection;
  VectorMap<Text> elements;
};

} // namespace datagui
