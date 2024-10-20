#include "datagui/element/checkbox.hpp"

namespace datagui {

void CheckboxSystem::calculate_size_components(
    Node& node,
    const Tree& tree) const
{
    node.fixed_size = Vecf::Constant(font.line_height * style.checkbox.size);
}

void CheckboxSystem::render(
    const Node& node,
    const NodeState& state,
    Renderers& renderers) const
{
    const auto& element = elements[node.element_index];
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

    if (!element.checked) {
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

void CheckboxSystem::release(const Node& node, const Vecf& mouse_pos) {
    auto& element = elements[node.element_index];
    element.checked = !element.checked;
    element.changed = true;
}

void CheckboxSystem::key_event(const Node& node, const KeyEvent& event) {
    auto& element = elements[node.element_index];
    if (!event.is_text && event.key_value == KeyValue::Enter) {
        element.checked = !element.checked;
        element.changed = true;
    }
}

} // namespace datagui
