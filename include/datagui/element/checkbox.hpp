#pragma once

#include "datagui/internal/element.hpp"
#include "datagui/internal/vector_map.hpp"
#include "datagui/style.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/tree.hpp"


namespace datagui {

struct Checkbox {
    bool checked;
    bool changed;

    Checkbox(bool default_checked):
        checked(default_checked),
        changed(false)
    {}
};

class CheckboxSystem: public ElementSystem {
public:
    CheckboxSystem(
        const Style& style,
        const FontStructure& font
    ):
        style(style),
        font(font)
    {}

    int create(bool default_checked) {
        return elements.emplace(default_checked);
    }

    void pop(int index) override {
        elements.pop(index);
    }

    const bool* query(const Node& node) {
        auto& element = elements[node.element_index];
        if (element.changed) {
            element.changed = false;
            return &element.checked;
        }
        return nullptr;
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
    VectorMap<Checkbox> elements;
};

} // namespace datagui
