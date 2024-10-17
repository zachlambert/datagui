#include "datagui/element/button.hpp"
#include "datagui/internal/text_renderer.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Button& element)
{
    node.fixed_size = text_size(font, element.text, element.max_width);
    node.fixed_size += Vecf::Constant(
        2 * (style.element.border_width + style.element.padding));
}

} // namespace datagui
