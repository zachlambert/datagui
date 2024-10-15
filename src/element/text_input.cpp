#include "datagui/element/text_input.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const TextRenderer& text_renderer,
    Node& node,
    const TextInput& element)
{
    node.fixed_size = text_renderer.text_size(
        element.text,
        element.max_width,
        style.text.line_height);
    node.fixed_size += Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
}

} // namespace datagui
