#include "datagui/element/button.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const TextRenderer& text_renderer,
    Node& node,
    const Button& element)
{
    node.fixed_size = text_renderer.text_size(
        element.text,
        element.max_width);
    node.fixed_size += Vecf::Constant((style.element.border_width + style.element.padding) * 2);
}

} // namespace datagui
