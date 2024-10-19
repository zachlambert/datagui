#pragma once

#include "datagui/style.hpp"
#include "datagui/internal/tree.hpp"
#include "datagui/internal/text.hpp"

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

} // namespace datagui
