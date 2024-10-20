#include "datagui/element/text.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void TextSystem::calculate_size_components(
    Node& node,
    const Tree& tree) const
{
    const auto& element = elements[node.element_index];
    if (element.max_width >= 0) {
        node.fixed_size = text_size(font, element.text, element.max_width);
    } else {
        node.dynamic_size.x = -element.max_width;
    }
}

void TextSystem::render(
    const Node& node,
    const NodeState& state,
    Renderers& renderers) const
{
    const auto& element = elements[node.element_index];

    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin+node.size),
        Color::Clear(),
        0,
        Color::Black(),
        0
    );

    if (state.focused) {
        render_selection(
            style,
            font,
            element.text,
            element.max_width,
            node.origin,
            text_selection,
            false,
            renderers.geometry
        );
    }

    renderers.text.queue_text(
        font,
        style.text.font_color,
        element.text,
        element.max_width,
        node.origin
    );
}

void TextSystem::press(const Node& node, const Vecf& mouse_pos) {
    const auto& element = elements[node.element_index];
    text_selection.reset(find_cursor(
        font,
        element.text,
        element.max_width,
        mouse_pos - node.origin
    ));
}

void TextSystem::held(const Node& node, const Vecf& mouse_pos) {
    const auto& element = elements[node.element_index];
    text_selection.end = find_cursor(
        font,
        element.text,
        element.max_width,
        mouse_pos - node.origin
    );
}

} // namespace datagui
