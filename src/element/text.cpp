#include "datagui/element/text.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

#if 0
void TextSystem::calculate_size_components(Node& node, const Tree& tree) const {
  const auto& element = elements[node.element_index];
  if (element.max_width >= 0) {
    node.fixed_size = text_size(font, element.text, element.max_width);
  } else {
    node.dynamic_size.x = -element.max_width;
  }
}
#endif

void TextSystem::render(const State& state, Renderers& renderers) const {
  const auto& element = elements[state.element_index];

#if 0
  if (state.in_focus_tree) {
    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin + node.size),
        Color::Clear(),
        style.element.border_width,
        style.element.focus_color,
        0);
  } else {
    renderers.geometry.queue_box(
        Boxf(node.origin, node.origin + node.size),
        Color::Clear(),
        0,
        Color::Black(),
        0);
  }

  if (state.focused) {
    render_selection(
        style,
        font,
        element.text,
        element.max_width,
        node.origin,
        text_selection,
        false,
        renderers.geometry);
  }

  renderers.text
      .queue_text(font, style.text.font_color, element.text, element.max_width, node.origin);
#endif
}

} // namespace datagui
