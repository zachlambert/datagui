#include "datagui/element/checkbox.hpp"

namespace datagui {

void calculate_size_components(
    const Tree& tree,
    const Style& style,
    Node& node,
    const Checkbox& element)
{
    node.fixed_size = Vecf::Constant(style.text.font_size * style.checkbox.size);
}

} // namespace datagui
