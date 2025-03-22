#include "datagui/element/text.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

void TextSystem::set_layout_input(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  node->fixed_size = font_manager.text_size(
      style.font,
      style.font_size,
      element.text,
      style.max_width);
}

void TextSystem::render(Tree::ConstPtr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

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
#endif

  text_renderer.queue_text(
      element.text,
      node->position,
      style.font,
      style.font_size,
      style.text_color,
      style.max_width);
}

} // namespace datagui
