#include "datagui/element/button.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {
void Button::calculate_size_components(
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Tree& tree) const
{
    node.fixed_size = text_size(font, text, max_width);
    node.fixed_size += Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
}

void Button::render(
    const Style& style,
    const FontStructure& font,
    const Node& node,
    const NodeState& state,
    const TextSelection& text_selection,
    Renderers& renderers) const
{
    const Color& bg_color =
        state.held
            ? style.element.pressed_bg_color
            :style.element.bg_color;
    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin+node.size),
        bg_color,
        style.element.border_width,
        style.element.border_color
    );
    renderers.text.queue_text(
        font,
        style.text.font_color,
        text,
        max_width,
        node.origin + Vecf::Constant(
            style.element.border_width + style.element.padding)
    );
}

} // namespace datagui
