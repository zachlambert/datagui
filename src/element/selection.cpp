#include "datagui/element/selection.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void SelectionSystem::calculate_size_components(
    Node& node,
    const Tree& tree) const
{
    const auto& element = elements[node.element_index];
    node.fixed_size = Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));

    node.fixed_size.y += font.line_height;
    if (element.max_width >= 0) {
        node.fixed_size.x += element.max_width;
    } else {
        node.dynamic_size.x = -element.max_width;
    }
}

void SelectionSystem::render(
    const Node& node,
    const NodeState& state,
    Renderers& renderers) const
{
    const auto& element = elements[node.element_index];
    const Color& border_color =
        state.focused
            ? style.element.focus_color
            : style.element.border_color;

    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin+node.size),
        style.element.bg_color,
        style.element.border_width,
        border_color
    );
    if (element.choice >= 0 && element.choice < element.choices.size()) {
        renderers.text.queue_text(
            font,
            style.text.font_color,
            element.choices[element.choice],
            element.max_width,
            node.origin + Vecf::Constant(
                style.element.border_width + style.element.padding)
        );
    }
    if (state.focused) {
        float offset = node.size.y;
        for (const auto& choice: element.choices) {
            Vecf origin = node.origin + Vecf(0, offset);
            offset += node.size.y;
            renderers.geometry.queue_box(
                Boxf(origin, origin+node.size),
                style.element.bg_color,
                style.element.border_width,
                style.element.border_color
            );
            renderers.text.queue_text(
                font,
                style.text.font_color,
                choice,
                element.max_width,
                origin + Vecf::Constant(
                    style.element.border_width + style.element.padding)
            );
        }
    }
}

bool SelectionSystem::release(const Node& node, const Vecf& mouse_pos) {
    // TODO
    return false;
}

} // namespace datagui
