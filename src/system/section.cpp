#include "datagui/system/section.hpp"

namespace datagui {

void SectionSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& section = element.section();

  section.header_size = fm->text_size(
                            section.label,
                            theme->text_font,
                            theme->text_size,
                            LengthWrap()) +
                        Vecf::Constant(2 * theme->text_padding);
  if (section.border) {
    section.header_size += Vecf::Constant(2 * theme->layout_border_width);
  }
  state.fixed_size = section.header_size;

  state.dynamic_size.x = 1;
  state.dynamic_size.y = 0;
  state.floating = false;

  auto child = element.child();
  if (!child) {
    return;
  }

  Vecf content_size = child.state().fixed_size;
  if (!section.tight) {
    content_size += 2.f * Vecf::Constant(theme->layout_outer_padding);
  }

  state.fixed_size.x = std::max(state.fixed_size.x, content_size.x);
  if (section.open) {
    state.fixed_size.y += content_size.y;
  }
}

void SectionSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  const auto& section = element.section();

  auto child = element.child();
  if (!child) {
    state.child_mask = state.box();
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

  child.state().position = state.position + Vecf(0, section.header_size.y);
  if (!section.tight) {
    child.state().position += Vecf::Constant(theme->layout_outer_padding);
  }

  Vecf available_size = state.size;
  available_size.y -= section.header_size.y;
  if (!section.tight) {
    available_size -= Vecf::Constant(2 * theme->layout_outer_padding);
  }

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

  state.child_mask = child.state().box();
}

void SectionSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& section = element.section();

  Color header_color =
      section.header_color ? *section.header_color : theme->layout_color_bg;
  int border_width = section.border ? theme->layout_border_width : 0;
  if (section.open) {
    header_color.r += 0.5 * (1 - header_color.r);
    header_color.g += 0.5 * (1 - header_color.g);
    header_color.b += 0.5 * (1 - header_color.b);
  }

  if (section.bg_color) {
    renderer.queue_box(state.box(), *section.bg_color, 0, Color::Black(), 0);
  }

  renderer.queue_box(
      Boxf(
          state.position,
          state.position + Vecf(state.size.x, section.header_size.y)),
      header_color,
      border_width,
      theme->layout_border_color,
      0);

  Vecf text_origin =
      state.position + Vecf::Constant(theme->text_padding + border_width);
  renderer.queue_text(
      text_origin,
      section.label,
      theme->text_font,
      theme->text_size,
      theme->text_color,
      LengthWrap());
}

bool SectionSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& section = element.section();

  Boxf header_box(
      state.position,
      state.position + Vecf(state.size.x, section.header_size.y));

  if (!header_box.contains(event.position)) {
    return false;
  }

  if (event.action == MouseAction::Release &&
      event.button == MouseButton::Left) {
    section.open = !section.open;
  }
  return false;
}

bool SectionSystem::key_event(ElementPtr element, const KeyEvent& event) {
  auto& section = element.section();

  if (event.action == KeyAction::Release && event.key == Key::Enter) {
    section.open = !section.open;
  }
  return false;
}

} // namespace datagui
