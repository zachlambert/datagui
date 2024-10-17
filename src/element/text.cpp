#include "datagui/element/text.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Text& element)
{
    if (element.max_width > 0) {
        node.fixed_size = text_size(font, element.text, element.max_width);
    } else {
        node.dynamic_size.x = -element.max_width;
    }
}

} // namespace datagui
