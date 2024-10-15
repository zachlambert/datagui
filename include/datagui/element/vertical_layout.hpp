#pragma once

#include "datagui/element.hpp"
#include "datagui/style.hpp"


namespace datagui {

struct VerticalLayout {
    Vecf input_size;

    VerticalLayout(
        const Vecf& input_size
    ):
        input_size(input_size)
    {}
};

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    Node& node,
    const VerticalLayout& element);

void calculate_child_sizes(
    Tree& tree,
    const Style& style,
    const Node& node,
    const VerticalLayout& element);

} // namespace datagui
