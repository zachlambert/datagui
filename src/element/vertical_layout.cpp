#include "datagui/element/vertical_layout.hpp"

namespace datagui {

void VerticalLayout::calculate_size_components(
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Tree& tree) const
{
    node.fixed_size = Vecf::Zero();
    node.dynamic_size = Vecf::Zero();

    // X direction
    if (input_size.x == 0) {
        int child = node.first_child;
        int count = 0;
        while (child != -1) {
            count++;
            node.fixed_size.x = std::max(node.fixed_size.x, tree[child].fixed_size.x);
            node.dynamic_size.x = std::max(node.dynamic_size.x, tree[child].dynamic_size.x);
            child = tree[child].next;
        }

    } else if (input_size.x > 0) {
        node.fixed_size.x = input_size.x;
    } else {
        node.dynamic_size.x = -input_size.x;
    }

    // Y direction
    if (input_size.y == 0) {
        int child = node.first_child;
        int count = 0;
        while (child != -1) {
            count++;
            node.fixed_size.y += tree[child].fixed_size.y;
            node.dynamic_size.y += tree[child].dynamic_size.y;
            child = tree[child].next;
        }
        node.fixed_size.y += (count - 1) * style.element.padding;

    } else if (input_size.y > 0) {
        node.fixed_size.y = input_size.y;
    } else {
        node.dynamic_size.y = -input_size.y;
    }

    node.fixed_size += Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
}

void VerticalLayout::calculate_child_dimensions(
    const Style& style,
    const Node& node,
    Tree& tree) const
{
    Vecf available = node.size - node.fixed_size;
    Vecf offset = Vecf::Constant(
        style.element.padding + style.element.border_width);

    int child_index = node.first_child;
    while (child_index != -1) {
        auto& child = tree[child_index];

        child.size = child.fixed_size;
        if (child.dynamic_size.x > 0){
            child.size.x = node.size.x - 2 * (style.element.padding + style.element.border_width);
        }
        if (child.dynamic_size.y > 0){
            child.size.y += (child.dynamic_size.y / node.dynamic_size.y) * available.y;
        }
        child.origin = node.origin + offset;

        offset.y += child.size.y + style.element.padding;
        child_index = child.next;
    }
}

void VerticalLayout::render(
    const Style& style,
    const FontStructure& font,
    const Node& node,
    const NodeState& state,
    const TextSelection& text_selection,
    Renderers& renderers) const
{
    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin+node.size),
        Color::Clear(),
        style.element.border_width,
        style.element.border_color,
        0
    );
}

} // namespace datagui
