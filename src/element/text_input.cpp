#include "datagui/element/text_input.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void TextInput::calculate_size_components(
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Tree& tree) const
{
    node.fixed_size = Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));

    if (max_width >= 0) {
        node.fixed_size += text_size(font, text_, max_width);
    } else {
        node.fixed_size.y += font.line_height;
        node.dynamic_size.x = -max_width;
    }
}

void TextInput::render(
    const Style& style,
    const FontStructure& font,
    const Node& node,
    const NodeState& state,
    const TextSelection& text_selection,
    Renderers& renderers) const
{
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
            text_,
            max_width,
            text_origin,
            text_selection,
            true,
            renderers.geometry);
    }

    renderers.text.queue_text(
        font,
        style.text.font_color,
        text_,
        max_width,
        text_origin
    );
}

} // namespace datagui
