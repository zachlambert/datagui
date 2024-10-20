#pragma once

#include <string>
#include "datagui/internal/element.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/tree.hpp"


namespace datagui {

struct Button {
    std::string text;
    float max_width;
    bool clicked;

    Button(const std::string& text, float max_width):
        text(text),
        max_width(max_width),
        clicked(false)
    {}
};

class ButtonSystem: public ElementSystem {
public:
    ButtonSystem(
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

    bool query(const Node& node) {
        auto& element = elements[node.element_index];
        if (element.clicked) {
            element.clicked = false;
            return true;
        }
        return false;
    }

    void calculate_size_components(
        Node& node,
        const Tree& tree) const override;

    void render(
        const Node& node,
        const NodeState& state,
        Renderers& renderers) const override;

    void release(
        const Node& node,
        const Vecf& mouse_pos) override;

    void key_event(const Node& node, const KeyEvent& event) override;

private:
    const Style& style;
    const FontStructure& font;
    VectorMap<Button> elements;
};

} // namespace datagui
