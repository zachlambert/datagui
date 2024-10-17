#include "datagui/element/horizontal_layout.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    Node& node,
    const HorizontalLayout& element)
{
    if (element.input_size.x == 0) {
        int child = node.first_child;
        int count = 0;
        while (child != -1) {
            count++;
            node.fixed_size.x += tree[child].fixed_size.x;
            node.dynamic_size.x += tree[child].dynamic_size.x;
            child = tree[child].next;
        }
        node.fixed_size.x += (count - 1) * style.element.padding;

    } else if (element.input_size.x > 0) {
        node.fixed_size.x = element.input_size.x;
    } else {
        node.dynamic_size.x = -element.input_size.x;
    }

    // Y direction
    if (element.input_size.y == 0) {
        int child = node.first_child;
        int count = 0;
        while (child != -1) {
            count++;
            node.fixed_size.y = std::max(node.fixed_size.y, tree[child].fixed_size.y);
            node.dynamic_size.y = std::max(node.dynamic_size.y, tree[child].dynamic_size.y);
            child = tree[child].next;
        }

    } else if (element.input_size.y > 0) {
        node.fixed_size.y = element.input_size.y;
    } else {
        node.dynamic_size.y = -element.input_size.y;
    }

    node.fixed_size += Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
}

void calculate_child_dimensions(
    Tree& tree,
    const Style& style,
    const Node& node,
    const HorizontalLayout& element)
{
    Vecf available = node.size - node.fixed_size;
    Vecf offset = Vecf::Constant(
        style.element.padding + style.element.border_width);

    int child_index = node.first_child;
    while (child_index != -1) {
        auto& child = tree[child_index];

        child.size = child.fixed_size;
        if (child.dynamic_size.x > 0){
            child.size.x += (child.dynamic_size.x / node.dynamic_size.x) * available.x;
        }
        if (child.dynamic_size.y > 0){
            child.size.y = node.size.y - 2 * (style.element.padding + style.element.border_width);
        }
        child.origin = node.origin + offset;

        offset.x += child.size.x + style.element.padding;
        child_index = child.next;
    }
}

} // namespace datagui
