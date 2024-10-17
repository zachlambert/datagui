#include "datagui/element/text_input.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const FontStructure& font,
    Node& node,
    const TextInput& element)
{
    node.fixed_size = Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));

    if (element.max_width >= 0) {
        node.fixed_size += text_size(font, element.text, element.max_width);
    } else {
        node.fixed_size.y += font.line_height;
        node.dynamic_size.x = -element.max_width;
    }
}

} // namespace datagui
