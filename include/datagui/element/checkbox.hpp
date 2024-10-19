#pragma once

#include "datagui/style.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/text.hpp"
#include "datagui/internal/renderers.hpp"


namespace datagui {

struct Checkbox {
    bool checked;

    Checkbox(bool default_checked = false):
        checked(default_checked)
    {}
};

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Checkbox& element);

void render_element(
    const Node& node,
    const Checkbox& element,
    const NodeState& state,
    const Style& style,
    const FontStructure& font,
    Renderers& renderers);

} // namespace datagui
