#include "datagui/system/section.hpp"

namespace datagui {

void SectionSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  const auto& section = element.section();

  Vecf header_size = fm->text_size(
                         section.label,
                         theme->text_font,
                         theme->text_size,
                         LengthWrap()) +
                     Vecf::Constant(2 * theme->text_padding);
  state.fixed_size = header_size;

  state.dynamic_size.x = 1;
  state.dynamic_size.y = 0;
  state.floating = false;

  auto child = element.child();
  if (!child || !section.open) {
    return;
  }

  state.fixed_size.x = std::max(
      state.fixed_size.x,
      child.state().fixed_size.x + 2 * theme->layout_outer_padding);
  state.fixed_size.y +=
      child.state().fixed_size.y + 2 * theme->layout_outer_padding;
}

void SectionSystem::set_dependent_state(ElementPtr element) {
  const auto& state = element.state();
  const auto& section = element.section();

  auto child = element.child();
  if (!child) {
    return;
  }
  for (auto other = child.next(); other; other = other.next()) {
    other.state().hidden = true;
  }

  if (!section.open) {
    child.state().hidden = true;
    return;
  }
  child.state().hidden = false;

  float header_height = fm->text_height(theme->text_font, theme->text_size) +
                        2 * theme->text_padding;

  child.state().position = state.position + Vecf(0, header_height) +
                           Vecf::Constant(theme->layout_outer_padding);

  Vecf available_size = state.size;
  available_size.y -= header_height;
  available_size -= Vecf::Constant(2 * theme->layout_outer_padding);

  if (child.state().dynamic_size.x > 0) {
    child.state().size.x = available_size.x;
  } else {
    child.state().size.x = child.state().fixed_size.x;
  }
  if (child.state().dynamic_size.y > 0) {
    child.state().size.y = available_size.y;
  } else {
    child.state().size.y = child.state().fixed_size.y;
  }
}

void SectionSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& section = element.section();

  Vecf header_size = fm->text_size(
                         section.label,
                         theme->text_font,
                         theme->text_size,
                         LengthWrap()) +
                     Vecf::Constant(2 * theme->text_padding);

  const Color& bg_color =
      section.open ? theme->input_color_bg_active : theme->input_color_bg;

  renderer.queue_box(
      Boxf(state.position, state.position + header_size),
      bg_color,
      0,
      Color::Black(),
      0);
  renderer.queue_text(
      state.position + Vecf::Constant(theme->text_padding),
      section.label,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

bool SectionSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& section = element.section();

  Vecf header_size = fm->text_size(
                         section.label,
                         theme->text_font,
                         theme->text_size,
                         LengthWrap()) +
                     Vecf::Constant(2 * theme->text_padding);
  Boxf header_box(state.position, state.position + header_size);

  if (!header_box.contains(event.position)) {
    return false;
  }

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    section.open = !section.open;
    return true;
  }
  return false;
}

bool SectionSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& section = element.section();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    section.open = !section.open;
    return true;
  }
  return false;
}

} // namespace datagui
