#include "datagui/element/checkbox.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    const FontStructure& font,
    Node& node,
    const Checkbox& element)
{
    node.fixed_size = Vecf::Constant(font.line_height * style.checkbox.size);
}

} // namespace datagui
