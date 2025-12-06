#include "datagui/system/select.hpp"

namespace datagui {

void SelectSystem::set_input_state(ElementPtr element) {
  auto& state = element.state();
  auto& select = element.select();

  if (!select.label.empty()) {
    select.label_size = fm->text_size(
                            select.label,
                            theme->text_font,
                            theme->text_size,
                            LengthWrap()) +
                        Vec2::uniform(2 * theme->text_padding);
  } else {
    select.label_size = Vec2();
  }

  float max_item_width = theme->select_min_width;
  for (const auto& choice : select.choices) {
    Vec2 choice_size =
        fm->text_size(choice, theme->text_font, theme->text_size, LengthWrap());
    max_item_width = std::max(max_item_width, choice_size.x);
  }

  float padding = theme->text_padding + theme->input_border_width;
  state.fixed_size.x = select.label_size.x + max_item_width + 2 * padding;
  state.fixed_size.y =
      fm->text_height(theme->text_font, theme->text_size) + 2 * padding;
  state.dynamic_size = Vec2();

  if (!select.choices.empty()) {
    state.floating = select.open;

    Vec2 floating_size;
    floating_size.x = max_item_width + 2 * padding;
    floating_size.y = select.choices.size() *
                          fm->text_height(theme->text_font, theme->text_size) +
                      (select.choices.size() + 1) *
                          (theme->input_border_width + theme->text_padding);

    Vec2 floating_offset;
    floating_offset.x = select.label_size.x;
    floating_offset.y = state.fixed_size.y;

    state.floating_type = FloatingTypeRelative(floating_offset, floating_size);
  } else {
    state.floating = false;
  }
}

void SelectSystem::set_dependent_state(ElementPtr element) {
  auto& state = element.state();
  auto& select = element.select();

  select.select_box = state.box();
  select.select_box.lower.x += select.label_size.x;
}

void SelectSystem::render(ConstElementPtr element, Renderer& renderer) {
  const auto& state = element.state();
  const auto& select = element.select();

  if (!select.label.empty()) {
    Vec2 offset(
        theme->text_padding,
        theme->text_padding + theme->input_border_width);
    renderer.queue_text(
        state.position + offset,
        select.label,
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap());
  }

  const Color& select_color =
      select.open ? theme->input_color_bg_active : theme->input_color_bg;
  renderer.queue_box(
      select.select_box,
      select_color,
      theme->input_border_width,
      theme->input_color_border);

  if (!select.choices.empty() && select.choice >= 0) {
    Vec2 offset =
        Vec2(select.label_size.x, 0) +
        Vec2::uniform(theme->text_padding + theme->input_border_width);
    renderer.queue_text(
        select.select_box.lower + offset,
        select.choices[select.choice],
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthWrap());
  }

  if (!select.open) {
    return;
  }

  Vec2 position = state.float_box.lower;
  Vec2 size = select.select_box.size();
  for (int i = 0; i < select.choices.size(); i++) {
    const Color& bg_color = (i == select.choice) ? theme->input_color_bg_active
                                                 : theme->input_color_bg;

    renderer.queue_box(
        Box2(position, position + size),
        bg_color,
        theme->input_border_width,
        theme->input_color_border);

    renderer.queue_text(
        position +
            Vec2::uniform(theme->input_border_width + theme->text_padding),
        select.choices[i],
        theme->text_font,
        theme->text_size,
        theme->text_color,
        LengthFixed(
            state.size.x -
            2.f * (theme->input_border_width + theme->text_padding)));

    position.y += theme->input_border_width + 2 * theme->text_padding +
                  fm->text_height(theme->text_font, theme->text_size);
  }
}

void SelectSystem::mouse_event(ElementPtr element, const MouseEvent& event) {
  const auto& state = element.state();
  auto& select = element.select();

  if (event.action != MouseAction::Press) {
    return;
  }

  if (!select.open) {
    select.open = true;
    return;
  }
  if (select.choices.empty()) {
    return;
  }

  Vec2 offset;
  int clicked = -1;

  for (int i = 0; i < select.choices.size(); i++) {
    Vec2 position = state.position + offset;
    Vec2 size = Vec2(
        state.size.x,
        fm->text_height(theme->text_font, theme->text_size) +
            2.f * (theme->text_padding + theme->input_border_width));

    if (Box2(position, position + size).contains(event.position)) {
      clicked = i;
      break;
    }

    offset.y += theme->input_border_width + 2.f * theme->text_padding +
                fm->text_height(theme->text_font, theme->text_size);
  }

  if (clicked != -1) {
    select.open = false;
    if (select.choice != clicked) {
      select.choice = clicked;
      if (select.callback) {
        select.callback(select.choice);
      } else {
        element.set_dirty();
      }
    }
  }
}

void SelectSystem::focus_enter(ElementPtr element) {
  auto& select = element.select();
  select.open = true;
}

bool SelectSystem::focus_leave(ElementPtr element, bool success) {
  auto& select = element.select();
  select.open = false;
  return false;
}

} // namespace datagui
