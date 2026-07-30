#include "datagui/system/dropdown.hpp"
#include "datagui/system_utils/layout.hpp"

namespace dgui {

void DropdownSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& dropdown = element.dropdown();

  const auto& font =
      font_registry->get_font(theme->text_font, theme->text_size);
  state.fixed_size =
      font.text_size(dropdown.label, LengthWrap()) +
      2 * Vec2::uniform(theme->input_border_width + theme->text_padding);

  state.dynamic_size.x = 0;
  state.dynamic_size.y = 0;

  layout_set_input_state(
      element,
      theme,
      dropdown.layout,
      dropdown.layout_state);

  state.floating = dropdown.open;
  state.float_only = false;

  if (state.floating) {
    Vec2 offset;
    if (dropdown.direction == Direction::Horizontal) {
      offset = Vec2(state.fixed_size.x, 0);
    } else {
      offset = Vec2(0, state.fixed_size.y);
    }
    state.floating_type =
        FloatingTypeRelative(offset, dropdown.layout_state.content_fixed_size);
  }
}

void DropdownSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& dropdown = element.dropdown();

  if (!dropdown.open) {
    return;
  }

  dropdown.layout_state.content_box = state.float_box;
  layout_set_dependent_state(
      element,
      theme,
      dropdown.layout,
      dropdown.layout_state);
}

void DropdownSystem::render(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& dropdown = element.dropdown();

  const Color& bg_color =
      dropdown.open ? theme->input_color_bg_active : theme->input_color_bg;
  dl.draw_box(
      state.box(),
      bg_color,
      theme->input_border_width,
      theme->input_color_border);

  Vec2 text_offset =
      Vec2::uniform(theme->input_border_width + theme->text_padding);
  dl.draw_text(
      font_registry->get_font(theme->text_font, theme->text_size),
      state.position + text_offset,
      theme->text_color,
      LengthWrap(),
      dropdown.label);

  if (dropdown.open) {
    dl.draw_box(state.float_box, theme->layout_color_bg);
    layout_render(dropdown.layout_state, theme, dl);
  }
}

void DropdownSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  auto& dropdown = element.dropdown();
  if (event.action == MouseAction::Press) {
    dropdown.open = true;
  }
}

bool DropdownSystem::scroll_event(
    ElementPtr element,
    const ScrollEvent& event) {
  auto& dropdown = element.dropdown();
  if (!dropdown.open) {
    return false;
  }
  return layout_scroll_event(dropdown.layout_state, event);
}

void DropdownSystem::focus_enter(ElementPtr element) {
  auto& dropdown = element.dropdown();
  dropdown.open = true;
}

void DropdownSystem::focus_tree_leave(ElementPtr element) {
  auto& dropdown = element.dropdown();
  dropdown.open = false;
}

} // namespace dgui
