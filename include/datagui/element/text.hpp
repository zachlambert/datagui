#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/selection.hpp"
#include "datagui/internal/tree.hpp"

namespace datagui {

struct Text {
    std::string text;
    float max_width;

    Text(const std::string& text, float max_width):
        text(text), max_width(max_width)
    {}
};

class TextSystem: public ElementSystem {
public:
    TextSystem(
        const Style& style,
        const FontStructure& font
    ):
        style(style),
        font(font)
    {}

    int create(const std::string& text, float max_width) {
        return elements.emplace(text, max_width);
    }

    void pop(int index) override {
        elements.pop(index);
    }

    void calculate_size_components(
        Node& node,
        const Tree& tree) const override;

    void render(
        const Node& node,
        const NodeState& state,
        Renderers& renderers) const override;

    void press(
        const Node& node,
        const Vecf& mouse_pos) override;

    void held(
        const Node& node,
        const Vecf& mouse_pos) override;

    void focus_enter(const Node& node) override;
    void key_event(const Node& node, const KeyEvent& event) override;

private:
    const Style& style;
    const FontStructure& font;
    TextSelection text_selection;
    VectorMap<Text> elements;
};

} // namespace datagui
