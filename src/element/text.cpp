#include "datagui/element/text.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void Text::calculate_size_components(
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Tree& tree) const
{
    if (max_width >= 0) {
        node.fixed_size = text_size(font, text, max_width);
    } else {
        node.dynamic_size.x = -max_width;
    }
}

void Text::render(
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
        0,
        Color::Black(),
        0
    );

    if (state.focused) {
        render_selection(
            style,
            font,
            text,
            max_width,
            node.origin,
            text_selection,
            false,
            renderers.geometry
        );
    }

    renderers.text.queue_text(
        font,
        style.text.font_color,
        text,
        max_width,
        node.origin
    );
}

} // namespace datagui
