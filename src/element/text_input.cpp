#include "datagui/element/text_input.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void TextInputSystem::calculate_size_components(
    Node& node,
    const Tree& tree) const
{
    const auto& element = elements[node.element_index];

    node.fixed_size = Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));

    if (element.max_width >= 0) {
        node.fixed_size += text_size(font, element.text, element.max_width);
    } else {
        node.fixed_size.y += font.line_height;
        node.dynamic_size.x = -element.max_width;
    }
}

void TextInputSystem::render(
    const Node& node,
    const NodeState& state,
    Renderers& renderers) const
{
    const auto& element = elements[node.element_index];

    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin+node.size),
        style.element.bg_color,
        style.element.border_width,
        state.focused
            ? style.element.focus_color
            : style.element.border_color
    );
    Vecf text_origin =
        node.origin
        + Vecf::Constant(
            style.element.border_width + style.element.padding);

    if (state.focused) {
        render_selection(
            style,
            font,
            element.text,
            element.max_width,
            text_origin,
            text_selection,
            true,
            renderers.geometry);
    }

    renderers.text.queue_text(
        font,
        style.text.font_color,
        element.text,
        element.max_width,
        text_origin
    );
}

void TextInputSystem::press(const Node& node, const Vecf& mouse_pos) {
    const auto& element = elements[node.element_index];
    Vecf text_origin = node.origin + Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
    text_selection.reset(find_cursor(
        font,
        element.text,
        element.max_width,
        mouse_pos - text_origin
    ));
}

void TextInputSystem::held(const Node& node, const Vecf& mouse_pos) {
    const auto& element = elements[node.element_index];
    Vecf text_origin = node.origin + Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
    text_selection.end = find_cursor(
        font,
        element.text,
        element.max_width,
        mouse_pos - text_origin
    );
}

} // namespace datagui
