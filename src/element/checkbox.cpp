#include "datagui/element/checkbox.hpp"

namespace datagui {

void Checkbox::calculate_size_components(
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Tree& tree) const
{
    node.fixed_size = Vecf::Constant(font.line_height * style.checkbox.size);
}

void Checkbox::render(
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
        : style.element.bg_color;

    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin+node.size),
        bg_color,
        style.element.border_width,
        style.element.border_color,
        0
    );

    if (!checked_) {
        return;
    }

    float offset = style.element.border_width + style.checkbox.check_padding;
    renderers.geometry.queue_box(
        Boxf(
            node.origin + Vecf::Constant(offset),
            node.origin + node.size - Vecf::Constant(offset)
        ),
        style.checkbox.check_color,
        0,
        Color::Black(),
        0
    );
}

} // namespace datagui
