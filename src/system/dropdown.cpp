#include "datagui/system/dropdown.hpp"
#include "datagui/system_utils/layout.hpp"

namespace datagui {

void DropdownSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& dropdown = element.dropdown();

  state.fixed_size =
      fm->text_size(
          dropdown.label,
          theme->text_font,
          theme->text_size,
          LengthWrap()) +
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

  state.child_mask = state.float_box;
  layout_set_dependent_state(
      element,
      state.float_box,
      theme,
      dropdown.layout,
      dropdown.layout_state);
}

void DropdownSystem::render(ConstElementPtr element, GuiRenderer& renderer) {
  const auto& state = element.state();
  const auto& dropdown = element.dropdown();

  const Color& bg_color =
      dropdown.open ? theme->input_color_bg_active : theme->input_color_bg;
  renderer.queue_box(
      state.box(),
      bg_color,
      theme->input_border_width,
      theme->input_color_border);

  Vec2 text_offset =
      Vec2::uniform(theme->input_border_width + theme->text_padding);
  renderer.queue_text(
      state.position + text_offset,
      dropdown.label,
      theme->text_font,
      theme->text_size,
      theme->text_color);

  if (dropdown.open) {
    renderer.queue_box(state.float_box, theme->layout_color_bg);
    layout_render_scroll(
        state.float_box,
        dropdown.layout_state,
        theme,
        renderer);
  }
}

void DropdownSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  auto& dropdown = element.dropdown();
  if (event.action == MouseAction::Press) {
    dropdown.open = true;
    element.set_dirty();
  }
}

bool DropdownSystem::scroll_event(
    ElementPtr element,
    const ScrollEvent& event) {
  const auto& state = element.state();
  auto& dropdown = element.dropdown();
  if (!dropdown.open) {
    return false;
  }
  return layout_scroll_event(state.float_box, dropdown.layout_state, event);
}

void DropdownSystem::focus_enter(ElementPtr element) {
  auto& dropdown = element.dropdown();
  dropdown.open = true;
  element.set_dirty();
}

void DropdownSystem::focus_tree_leave(ElementPtr element) {
  auto& dropdown = element.dropdown();
  dropdown.open = false;
  element.set_dirty();
}

} // namespace datagui
