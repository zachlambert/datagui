#include "datagui/element/button.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void ButtonSystem::calculate_size_components(
    Node& node,
    const Tree& tree) const
{
    const auto& element = elements[node.element_index];
    node.fixed_size = text_size(font, element.text, element.max_width);
    node.fixed_size += Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
}

void ButtonSystem::render(
    const Node& node,
    const NodeState& state,
    Renderers& renderers) const
{
    const auto& element = elements[node.element_index];
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
        element.text,
        element.max_width,
        node.origin + Vecf::Constant(
            style.element.border_width + style.element.padding)
    );
}

} // namespace datagui
