#pragma once

#include "datagui/element.hpp"
#include "datagui/style.hpp"

namespace datagui {

struct HorizontalLayout {
    Vecf input_size;

    HorizontalLayout(const Vecf& input_size):
        input_size(input_size)
    {}
};

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    Node& node,
    const HorizontalLayout& element);

void calculate_child_sizes(
    Tree& tree,
    const Style& style,
    const Node& node,
    const HorizontalLayout& element);

} // namespace datagui
