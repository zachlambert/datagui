#include "datagui/element/text.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const TextRenderer& text_renderer,
    Node& node,
    const Text& element)
{
    node.fixed_size = text_renderer.text_size(
        element.text,
        element.max_width,
        style.text.line_height);

}

} // namespace datagui
