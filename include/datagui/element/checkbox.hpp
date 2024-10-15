#pragma once

#include "datagui/element.hpp"
#include "datagui/style.hpp"

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
    Node& node,
    const Checkbox& element);

} // namespace datagui
