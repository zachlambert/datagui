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

  state.content_mode =
      dropdown.open ? DisplayMode::Float : DisplayMode::Disabled;
}

void DropdownSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& dropdown = element.dropdown();

  if (!dropdown.open) {
    return;
  }

  Vec2 offset;
  if (dropdown.direction == Direction::Horizontal) {
    offset = Vec2(state.fixed_size.x, 0);
  } else {
    offset = Vec2(0, state.fixed_size.y);
  }
  state.content_box = Box2::from_size(
      state.position + offset,
      dropdown.layout_state.content_fixed_size);

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
}

void DropdownSystem::render_content(ConstElementPtr element, DrawList& dl) {
  const auto& state = element.state();
  const auto& dropdown = element.dropdown();

  dl.draw_box(state.content_box, theme->layout_color_bg);
  layout_render_scroll(state.content_box, dropdown.layout_state, theme, dl);
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
  const auto& state = element.state();
  auto& dropdown = element.dropdown();
  if (!dropdown.open) {
    return false;
  }
  return layout_scroll_event(state.content_box, dropdown.layout_state, event);
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
