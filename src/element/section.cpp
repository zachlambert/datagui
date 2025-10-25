#include "datagui/element/section.hpp"

namespace datagui {

void SectionSystem::set_input_state(
    Element& e,
    const ConstElementList& children) {
  const auto& props = *e.props.cast<SectionProps>();

  Vecf header_size = fm->text_size(
                         props.label,
                         theme->text_font,
                         theme->text_size,
                         LengthWrap()) +
                     Vecf::Constant(2 * theme->text_padding);
  e.fixed_size = header_size;

  e.dynamic_size.x = 1;
  e.dynamic_size.y = 0;
  e.floating = false;

  if (children.empty() || !props.open) {
    return;
  }
  const auto& child = *children[0];
  e.fixed_size += Vecf::Constant(theme->layout_outer_padding);
  e.fixed_size.x = std::max(
      e.fixed_size.x,
      child.fixed_size.x + 2 * theme->layout_outer_padding);
  e.fixed_size.y += child.fixed_size.y + 2 * theme->layout_outer_padding;
}

void SectionSystem::set_dependent_state(
    Element& e,
    const ElementList& children) {

  if (children.empty()) {
    return;
  }

  // Always hide excess children
  for (std::size_t i = 1; i < children.size(); i++) {
    children[i]->hidden = true;
  }

  auto& child = *children[0];

  const auto& props = *e.props.cast<SectionProps>();
  if (!props.open) {
    child.hidden = true;
    return;
  }
  child.hidden = false;

  float header_height = fm->text_height(theme->text_font, theme->text_size) +
                        2 * theme->text_padding;

  child.position = e.position + Vecf(0, header_height) +
                   Vecf::Constant(theme->layout_outer_padding);

  Vecf available_size = e.size;
  available_size.y -= header_height;
  available_size -= Vecf::Constant(2 * theme->layout_outer_padding);

  if (child.dynamic_size.x > 0) {
    child.size.x = available_size.x;
  } else {
    child.size.x = child.fixed_size.x;
  }
  if (child.dynamic_size.y > 0) {
    child.size.y = available_size.y;
  } else {
    child.size.y = child.fixed_size.y;
  }
}

void SectionSystem::render(const Element& e, Renderer& renderer) {
  const auto& props = *e.props.cast<SectionProps>();

  Vecf header_size = fm->text_size(
                         props.label,
                         theme->text_font,
                         theme->text_size,
                         LengthWrap()) +
                     Vecf::Constant(2 * theme->text_padding);

  const Color& bg_color =
      props.open ? theme->input_color_bg_active : theme->input_color_bg;

  renderer.queue_box(
      Boxf(e.position, e.position + header_size),
      bg_color,
      0,
      Color::Black(),
      0);
  renderer.queue_text(
      e.position + Vecf::Constant(theme->text_padding),
      props.label,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

bool SectionSystem::mouse_event(Element& e, const MouseEvent& event) {
  auto& props = *e.props.cast<SectionProps>();

  Vecf header_size = fm->text_size(
                         props.label,
                         theme->text_font,
                         theme->text_size,
                         LengthWrap()) +
                     Vecf::Constant(2 * theme->text_padding);
  Boxf header_box(e.position, e.position + header_size);

  if (!header_box.contains(event.position)) {
    return false;
  }

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    props.open = !props.open;
    return true;
  }
  return false;
}

bool SectionSystem::key_event(Element& e, const KeyEvent& event) {
  auto& props = *e.props.cast<SectionProps>();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    props.open = !props.open;
    return true;
  }
  return false;
}

} // namespace datagui
