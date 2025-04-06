#include "datagui/element/text_box.hpp"
#include "datagui/visual/text_renderer.hpp"

namespace datagui {

void TextBoxSystem::set_layout_input(Tree::Ptr node) const {
  const auto& element = elements[node->element_index];
  const auto& style = element.style;

  node->fixed_size =
      (element.style.border_width + element.style.padding).size();

  Vecf text_size = font_manager.text_size(element.text, element.style);
  node->fixed_size.y += text_size.y;

  if (auto width = std::get_if<LengthFixed>(&element.style.text_width)) {
    node->fixed_size.x += width->value;
  } else {
    node->fixed_size.x += text_size.x;
    if (auto width = std::get_if<LengthDynamic>(&element.style.text_width)) {
      node->dynamic_size.x = width->weight;
    }
  }
}

void TextBoxSystem::render(Tree::ConstPtr node) const {
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

  text_renderer.queue_text(node->position, element.text, style);
}

} // namespace datagui
